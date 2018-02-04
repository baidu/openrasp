/*
 * Copyright 2017-2018 Baidu Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.fuxi.javaagent.plugin.checker.local;

import com.baidu.rasp.SQLLexer;
import org.antlr.v4.runtime.ANTLRInputStream;
import org.antlr.v4.runtime.Token;

import java.util.HashSet;
import java.util.concurrent.ConcurrentHashMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Created by fastdev@163.com on 2/4/18.
 * All rights reserved
 */
public class TrustStringManager {
    private static ThreadLocal<HashSet<CharSequence>> badStringParts = new ThreadLocal<HashSet<CharSequence>>();

    private static ThreadLocal<HashSet<CharSequence>> goodStrings = new ThreadLocal<HashSet<CharSequence>>();

    private static ThreadLocal<HashSet<CharSequence>> uncertainParts = new ThreadLocal<HashSet<CharSequence>>();

    private static ConcurrentHashMap<CharSequence, Object> constMap = new ConcurrentHashMap<CharSequence, Object>();

    public static void initRequest() {
        badStringParts.set(new HashSet<CharSequence>());
        goodStrings.set(new HashSet<CharSequence>());
        uncertainParts.set(new HashSet<CharSequence>());
    }

    public static void endRequest() {
        badStringParts.set(null);
        goodStrings.set(null);
        uncertainParts.set(null);
    }

    public static String getConstString(String str) {
        if(str != null) {
            constMap.put(str, "");
        }
        return str;
    }

    public static StringBuilder handleBuilderAdd(StringBuilder sb, CharSequence str) {
        if(!isValidatePart(str)) {
            addBadString(sb);
        }
        sb.append(str);
        return sb;
    }

    public static String handleBuilderToString(StringBuilder sb) {
        String strResult = sb.toString();
        HashSet<CharSequence> set = badStringParts.get();
        if(set != null && set.contains(sb)) {
            addBadString(strResult);
        }
        else {
            addGoodString(strResult);
        }
        return strResult;
    }

    public static void addBadString(CharSequence str) {
        HashSet<CharSequence> set = badStringParts.get();
        if(set != null && str != null) {
            set.add(str);
        }
    }

    private static void addGoodString(CharSequence str) {
        HashSet<CharSequence> set = goodStrings.get();
        if(set != null && str != null) {
            set.add(str);
        }
    }

    public static boolean isSqlValidate(String sql) {
        if(!isValidatePart(sql)) {
            return false;
        }
        HashSet<CharSequence> uncertainSet = uncertainParts.get();
        if(uncertainSet.isEmpty()) {
            return true;
        }
        for(CharSequence word : uncertainSet) {
            if(!isParamValueValidate(sql, word.toString())) {
                return false;
            }
        }
        return true;
    }

    private static boolean isParamValueValidate(String query, String paramValue) {
        if(query.indexOf(paramValue) < 0) {
            return true;
        }
        ANTLRInputStream input = new ANTLRInputStream(query);
        SQLLexer lexer = new SQLLexer(input);
        for(Token token = lexer.nextToken(); token.getType() != -1; token = lexer.nextToken()) {
            if(isStringType(token)) {
                String v = token.getText();
                if(v.length() <= 2) {
                    continue;
                }
                v = v.substring(1, v.length() - 1);
                if (v.indexOf(paramValue) >= 0) {
                    return true;
                }
            }
        }
        return false;
    }

    private static boolean isStringType(Token token) {
        return token.getType() == 5;
    }

    private static boolean isValidatePart(CharSequence str) {
        if(str == null || str == "") {
            return true;
        }

        if(isNumberic(str)) {
            return true;
        }

        if(constMap.containsKey(str)) {
            return true;
        }

        HashSet<CharSequence> set = goodStrings.get();
        if(set != null && set.contains(str)) {
            return true;
        }
        if(isSafeValue(str)) {
            HashSet<CharSequence> uncertainSet = uncertainParts.get();
            uncertainSet.add(str);
            return true;
        }
        return false;
    }


    public static boolean isNumberic(CharSequence v) {
        if(v == null) {
            return false;
        }
        Pattern pattern = Pattern.compile("^-?[0-9]+([.][0-9]+)*$");
        Matcher isNum = pattern.matcher(v);
        return isNum != null && isNum.find();
    }

    //id guid number
    public static boolean isSafeValue(CharSequence v) {
        if(v == null) {
            return false;
        }
        Pattern pattern = Pattern.compile("^(-|[.]|@|[?]|[\\u4e00-\\u9fa5a-zA-Z0-9])*$");
        Matcher matcher = pattern.matcher(v);
        return matcher != null && matcher.find();
    }
}
