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

import com.baidu.rasp.TokenGenerator;
import com.fuxi.javaagent.config.Config;
import com.fuxi.javaagent.plugin.antlrlistener.TokenizeErrorListener;

import java.util.IdentityHashMap;
import java.util.LinkedList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Created by fastdev@163.com on 2/4/18.
 * All rights reserved
 */
//undo:only judge in webapp work thread not back thread.unless will cause outofmemory...ss
//undo:call druid sqlparser(some problem with mysql). or sqlparser( antlr) donot distinct big/little string...(or by config)
/*
/com/alibaba/druid/sql/parser/Lexer.java的
1、scanString2_d()方法 1251行判断字符串结束好像有问题，感觉应该用双引号才对？
2、scanString 方法对 mysql的支持不全，mysql里面 \ 也是可以转义的。
antlr 区分大小写的？
* */

public class TrustStringManager {
    private static ThreadLocal<IdentityHashMap<CharSequence, CharSequence>> certainMap = new ThreadLocal<IdentityHashMap<CharSequence, CharSequence>>();
    private static ThreadLocal<IdentityHashMap<CharSequence, CharSequence>> uncertainMap = new ThreadLocal<IdentityHashMap<CharSequence, CharSequence>>();
    private static ThreadLocal<IdentityHashMap<CharSequence, LinkedList<UncertItem>>> uncertainPartsMap = new ThreadLocal<IdentityHashMap<CharSequence, LinkedList<UncertItem>>>();
    private static TokenizeErrorListener tokenizeErrorListener = new TokenizeErrorListener();

    private static class UncertItem
    {
        private CharSequence value;
        private boolean isCertain;
        public UncertItem(boolean isCertain, CharSequence value) {
            this.isCertain = isCertain;
            this.value = value;
        }
    }

    public static void initRequest() {
        certainMap.set(new IdentityHashMap<CharSequence, CharSequence>());
        uncertainMap.set(new IdentityHashMap<CharSequence, CharSequence>());
        uncertainPartsMap.set(new IdentityHashMap<CharSequence, LinkedList<UncertItem>>());
    }

    public static void endRequest() {
        certainMap.set(null);
        uncertainMap.set(null);
        uncertainPartsMap.set(null);
    }

    public static String getConstString(String str) {
        addValidateString(str);
        return str;
    }

    private static void addUncertParts(IdentityHashMap<CharSequence, LinkedList<UncertItem>> uncertPartsMap,
                                       CharSequence dest, boolean isCertain, CharSequence part) {
        if(part == null || part == "") {
            return;
        }
        LinkedList<UncertItem> list = uncertPartsMap.get(dest);
        if(list == null) {
            list = new LinkedList<UncertItem>();
            uncertPartsMap.put(dest, list);
        }
        list.addLast(new UncertItem(isCertain, part));
    }

    private static void checkIsAddValidate(StringBuilder sb, CharSequence str) {
        IdentityHashMap<CharSequence, CharSequence> certMap = certainMap.get();
        IdentityHashMap<CharSequence, CharSequence> uncertMap = uncertainMap.get();
        IdentityHashMap<CharSequence, LinkedList<UncertItem>> uncertPartsMap =  uncertainPartsMap.get();
        if(certMap == null || uncertMap == null || uncertPartsMap == null) {
            return;
        }

        if(str == null || str == "" || certMap.containsKey(str) || isNumberic(str)) {
            addUncertParts(uncertPartsMap, sb, true, str);
            return;
        }

        uncertMap.put(sb, sb);

        LinkedList<UncertItem> oldUncert = uncertPartsMap.get(str);
        if(oldUncert == null) {
            addUncertParts(uncertPartsMap, sb, false, str);
        }
        else {
            for(UncertItem item : oldUncert) {
                addUncertParts(uncertPartsMap, sb, item.isCertain, item.value);
            }
        }
    }

    public static StringBuilder handleBuilderAdd(StringBuilder sb, CharSequence str) {
        checkIsAddValidate(sb, str);
        sb.append(str);
        return sb;
    }

    public static StringBuilder handleBuilderAdd(StringBuilder sb, char v) {
        String str = String.valueOf(v);
        addValidateString(str);
        checkIsAddValidate(sb, str);
        sb.append(v);
        return sb;
    }

    private static void checkBuilder(StringBuilder sb, String strResult) {
        IdentityHashMap<CharSequence, CharSequence> certMap = certainMap.get();
        IdentityHashMap<CharSequence, CharSequence> uncertMap = uncertainMap.get();
        IdentityHashMap<CharSequence, LinkedList<UncertItem>> uncertPartsMap =  uncertainPartsMap.get();
        if(certMap == null || uncertMap == null || uncertPartsMap == null) {
            return;
        }
        if(uncertMap.containsKey(sb)) {
            uncertMap.put(strResult, strResult);
            LinkedList<UncertItem> linkedList = uncertPartsMap.get(sb);
            if(linkedList != null) {
                LinkedList<UncertItem> newList = new LinkedList<UncertItem>();
                newList.addAll(linkedList);
                uncertPartsMap.put(strResult, newList);
            }
        }
        else {
            certMap.put(strResult, strResult);
        }
    }

    public static String handleBuilderToString(StringBuilder sb) {
        String strResult = sb.toString();
        checkBuilder(sb, strResult);
        return strResult;
    }


    private static void addValidateString(CharSequence str) {
        IdentityHashMap<CharSequence, CharSequence> map = certainMap.get();
        if(map != null && str != null) {
            map.put(str, str);
        }
    }

    public static boolean isSqlValidate(String sql) {
        IdentityHashMap<CharSequence, CharSequence> certMap = certainMap.get();
        IdentityHashMap<CharSequence, CharSequence> uncertMap = uncertainMap.get();
        IdentityHashMap<CharSequence, LinkedList<UncertItem>> uncertPartsMap =  uncertainPartsMap.get();
        if(certMap == null || uncertMap == null || uncertPartsMap == null) {
            //log after dispose...
            return true;
        }

        if(sql == null || sql == "" || certMap.containsKey(sql)) { // || isNumberic(sql)
            return true;
        }

        LinkedList<UncertItem> uncertLink = uncertPartsMap.get(sql);
        if(uncertLink == null || uncertLink.size() == 0) {
            return false;
        }
        StringBuilder newBuilder = new StringBuilder();
        for(UncertItem item : uncertLink) {
            if(item.isCertain) {
                newBuilder.append(item.value);
            }
            else {
                newBuilder.append("0");
            }
        }
        boolean isOk = getTokenCount(sql) == getTokenCount(newBuilder.toString());
        if(isOk) {
            certMap.put(sql, sql);
            uncertMap.remove(sql);
            uncertPartsMap.remove(sql);
        }
        return isOk;
    }

    private static int getTokenCount(String sql) {
        String[] array = TokenGenerator.tokenize(sql, tokenizeErrorListener);
        int len = array == null ? 0 : array.length;
        return len;
    }


    public static boolean isNumberic(CharSequence v) {
        if(v == null) {
            return false;
        }
        Pattern pattern = Pattern.compile("^-?[0-9]+([.][0-9]+)*$");
        Matcher isNum = pattern.matcher(v);
        return isNum != null && isNum.find();
    }

    public static boolean isTrustStringConfiged() {
        String scanPackage = Config.getConfig().getSqlInjectScanClassPrefix();
        if(scanPackage == null || "".equals(scanPackage)) {
            return false;
        }
        return true;
    }
}