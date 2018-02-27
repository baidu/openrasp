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

import java.util.HashMap;
import java.util.HashSet;
import java.util.IdentityHashMap;
import java.util.concurrent.ConcurrentHashMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Created by fastdev@163.com on 2/4/18.
 * All rights reserved
 */
public class TrustStringManager {


    private static ThreadLocal<IdentityHashMap<CharSequence, CharSequence>> badStringParts = new ThreadLocal<IdentityHashMap<CharSequence, CharSequence>>();
    private static ThreadLocal<IdentityHashMap<CharSequence, CharSequence>> goodStrings = new ThreadLocal<IdentityHashMap<CharSequence, CharSequence>>();

    private static ThreadLocal<IdentityHashMap<CharSequence, UncertInputState>> uncertainParts = new ThreadLocal<IdentityHashMap<CharSequence, UncertInputState>>();
    private static ThreadLocal<IdentityHashMap<CharSequence, CharSequence>> constMap = new ThreadLocal<IdentityHashMap<CharSequence, CharSequence>>();

    public static void initRequest() {
        badStringParts.set(new IdentityHashMap<CharSequence, CharSequence>());
        goodStrings.set(new IdentityHashMap<CharSequence, CharSequence>());
        uncertainParts.set(new IdentityHashMap<CharSequence, UncertInputState>());
        constMap.set(new IdentityHashMap<CharSequence, CharSequence>());
    }

    public static void endRequest() {
        badStringParts.set(null);
        goodStrings.set(null);
        uncertainParts.set(null);
        constMap.set(null);
    }

    public static String getConstString(String str) {
        if(str != null) {
            IdentityHashMap<CharSequence, CharSequence> map = constMap.get();
            if(map != null) {
                map.put(str, str);
            }
        }
        return str;
    }

    private static boolean isCharAt(CharSequence str, int index, char value) {
        if(str == null) {
            return false;
        }
        int len = str.length();
        if(index >= 0 && index < len) {
            return str.charAt(index) == value;
        }
        return false;
    }

    private static void checkUncertainAdd(StringBuilder sb, InputValidateResultEnum validateResult, CharSequence str) {
        IdentityHashMap<CharSequence, UncertInputState> uncertainPart = uncertainParts.get();
        UncertInputState state = uncertainPart.get(sb);
        switch (state) {
            case NoQuote:
                if(validateResult.equals(InputValidateResultEnum.SafeString)) {
                    return;
                }

                if(validateResult.equals(InputValidateResultEnum.Ok)) {
                    if(isCharAt(str, 0, '\'')
                            || (isCharAt(str, 0, '%') && isCharAt(str, 1, '\''))) {
                        uncertainPart.put(sb, UncertInputState.RightQuote);
                        return;
                    }
                }
                addBadString(sb);
                break;
            case LeftQuote:
                if(validateResult.equals(InputValidateResultEnum.SafeString)) {
                    return;
                }

                if(validateResult.equals(InputValidateResultEnum.Ok)) {
                    if(isCharAt(str, 0, '\'')
                            || (isCharAt(str, 0, '%') && isCharAt(str, 1, '\''))) {
                        uncertainPart.remove(sb);
                        return;
                    }
                }
                addBadString(sb);
                break;
            case RightQuote:
                if(validateResult.equals(InputValidateResultEnum.Ok)) {
                    return;
                }
                addBadString(sb);
                break;
            default:
                break;

        }
    }

    private static void checkNormalAdd(StringBuilder sb, InputValidateResultEnum validateResult, CharSequence str) {
        if(validateResult.equals(InputValidateResultEnum.Ok)) {
            return;
        }

        int len = sb.length();
        if(validateResult.equals(InputValidateResultEnum.SafeString)) {
            IdentityHashMap<CharSequence, UncertInputState> uncertainPart = uncertainParts.get();
            UncertInputState state = uncertainPart.get(str);
            if(len == 0) {
                uncertainPart.put(sb, state);
                return;
            }
            switch(state)
            {
                case NoQuote:
                    if(isCharAt(sb, len - 1, '\'')
                            || (isCharAt(sb, len - 1, '%') && isCharAt(sb, len - 2, '\''))){
                        uncertainPart.put(sb, UncertInputState.LeftQuote);
                        return;
                    }
                    addBadString(sb);
                    break;
                case LeftQuote:
                    uncertainPart.put(sb, UncertInputState.LeftQuote);
                    break;
                case RightQuote:
                    if(isCharAt(sb, len - 1, '\'')
                            || (isCharAt(sb, len - 1, '%') && isCharAt(sb, len - 2, '\''))){
                        uncertainPart.remove(sb);
                        return;
                    }
                    addBadString(sb);
                    break;
                default:
                    break;
            }
        }
    }


    private static void checkIsAddValidate(StringBuilder sb, CharSequence str) {
        IdentityHashMap<CharSequence, CharSequence> set = badStringParts.get();
        if(set != null && set.containsKey(sb)) {
            return;
        }

        InputValidateResultEnum validateResult = isValidatePart(str);
        if(validateResult.equals(InputValidateResultEnum.Fail)) {
            addBadString(sb);
            return;
        }

        IdentityHashMap<CharSequence, UncertInputState> uncertainPart = uncertainParts.get();
        if(uncertainPart != null && uncertainPart.containsKey(sb)) {
            checkUncertainAdd(sb, validateResult, str);
            return;
        }
        checkNormalAdd(sb, validateResult, str);
    }

    public static StringBuilder handleBuilderAdd(StringBuilder sb, CharSequence str) {
        checkIsAddValidate(sb, str);
        sb.append(str);
        return sb;
    }

    public static StringBuilder handleBuilderAdd(StringBuilder sb, char v) {
        String str = String.valueOf(v);
        getConstString(str);
        checkIsAddValidate(sb, str);
        sb.append(v);
        return sb;
    }

    private static void checkBuilder(StringBuilder sb, String strResult) {
        IdentityHashMap<CharSequence, CharSequence> set = badStringParts.get();
        if(set != null && set.containsKey(sb)) {
            addBadString(strResult);
            return;
        }

        IdentityHashMap<CharSequence, UncertInputState> uncertMap = uncertainParts.get();
        if(uncertMap != null && uncertMap.containsKey(sb)) {
            uncertMap.put(strResult, uncertMap.get(sb));
            return;
        }

        addGoodString(strResult);
    }

    public static String handleBuilderToString(StringBuilder sb) {
        String strResult = sb.toString();
        checkBuilder(sb, strResult);
        return strResult;
    }

    public static void addBadString(CharSequence str) {
        IdentityHashMap<CharSequence, CharSequence> set = badStringParts.get();
        if(set != null && str != null) {
            set.put(str, str);
        }
    }

    private static void addGoodString(CharSequence str) {
        IdentityHashMap<CharSequence, CharSequence> set = goodStrings.get();
        if(set != null && str != null) {
            set.put(str, str);
        }
    }

    public static boolean isSqlValidate(String sql) {
        return isValidatePart(sql).equals(InputValidateResultEnum.Ok);
    }


    private static InputValidateResultEnum isValidatePart(CharSequence str) {
        if(str == null || str == "") {
            return InputValidateResultEnum.Ok;
        }

        if(isNumberic(str)) {
            return InputValidateResultEnum.Ok;
        }

        IdentityHashMap<CharSequence, CharSequence> constmap = constMap.get();
        if(constmap.containsKey(str)) {
            return InputValidateResultEnum.Ok;
        }

        IdentityHashMap<CharSequence, CharSequence> set = goodStrings.get();
        if(set != null && set.containsKey(str)) {
            return InputValidateResultEnum.Ok;
        }

        IdentityHashMap<CharSequence, UncertInputState> uncertMap = uncertainParts.get();
        if(uncertMap != null && uncertMap.containsKey(str)) {
            return InputValidateResultEnum.SafeString;
        }
        if(isSafeValue(str)) {
            uncertMap.put(str, UncertInputState.NoQuote);
            return InputValidateResultEnum.SafeString;
        }
        return InputValidateResultEnum.Fail;
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
        Pattern pattern = Pattern.compile("^(-|[.]|@|[\\u4e00-\\u9fa5a-zA-Z0-9])*$");
        Matcher matcher = pattern.matcher(v);
        return matcher != null && matcher.find();
    }


    private static enum InputValidateResultEnum
    {
        Ok,
        Fail,
        SafeString
    }

    private static enum UncertInputState
    {
        NoQuote,
        LeftQuote,
        RightQuote
    }
}