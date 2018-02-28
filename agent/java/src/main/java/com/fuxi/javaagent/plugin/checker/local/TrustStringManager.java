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
    private static ThreadLocal<IdentityHashMap<CharSequence, StringCheckStateEnum>> inputValidateMap = new ThreadLocal<IdentityHashMap<CharSequence, StringCheckStateEnum>>();

    public static void initRequest() {
        inputValidateMap.set(new IdentityHashMap<CharSequence, StringCheckStateEnum>());
    }

    public static void endRequest() {
        inputValidateMap.set(null);
    }

    public static String getConstString(String str) {
        addValidateString(str);
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

    private static void checkUncertainAdd(StringBuilder sb, StringCheckStateEnum validateResult, CharSequence str) {
        IdentityHashMap<CharSequence, StringCheckStateEnum> uncertainPart = inputValidateMap.get();
        StringCheckStateEnum state = uncertainPart.get(sb);
        switch (state) {
            case UncertainNoQuote:
                if(validateResult.equals(StringCheckStateEnum.Uncertain)) {
                    return;
                }

                if(validateResult.equals(StringCheckStateEnum.Validate)) {
                    if(isCharAt(str, 0, '\'')
                            || (isCharAt(str, 0, '%') && isCharAt(str, 1, '\''))) {
                        uncertainPart.put(sb, StringCheckStateEnum.UncertainRightQuote);
                        return;
                    }
                }
                addBadString(sb);
                break;
            case UncertainLeftQuote:
                if(validateResult.equals(StringCheckStateEnum.Uncertain)) {
                    return;
                }

                if(validateResult.equals(StringCheckStateEnum.Validate)) {
                    if(isCharAt(str, 0, '\'')
                            || (isCharAt(str, 0, '%') && isCharAt(str, 1, '\''))) {
                        addValidateString(sb);
                        return;
                    }
                }
                addBadString(sb);
                break;
            case UncertainRightQuote:
                if(validateResult.equals(StringCheckStateEnum.Validate)) {
                    return;
                }
                addBadString(sb);
                break;
            default:
                break;

        }
    }

    private static void checkNormalAdd(StringBuilder sb, StringCheckStateEnum validateResult, CharSequence str) {
        if(validateResult.equals(StringCheckStateEnum.Validate)) {
            addValidateString(sb);
            return;
        }

        int len = sb.length();
        if(validateResult.equals(StringCheckStateEnum.Uncertain)) {
            IdentityHashMap<CharSequence, StringCheckStateEnum> uncertainPart = inputValidateMap.get();
            StringCheckStateEnum state = uncertainPart.get(str);
            if(len == 0) {
                uncertainPart.put(sb, state);
                return;
            }
            switch(state)
            {
                case UncertainNoQuote:
                    if(isCharAt(sb, len - 1, '\'')
                            || (isCharAt(sb, len - 1, '%') && isCharAt(sb, len - 2, '\''))){
                        uncertainPart.put(sb, StringCheckStateEnum.UncertainLeftQuote);
                        return;
                    }
                    addBadString(sb);
                    break;
                case UncertainLeftQuote:
                    uncertainPart.put(sb, StringCheckStateEnum.UncertainLeftQuote);
                    break;
                case UncertainRightQuote:
                    if(isCharAt(sb, len - 1, '\'')
                            || (isCharAt(sb, len - 1, '%') && isCharAt(sb, len - 2, '\''))){
                        addValidateString(sb);
                        return;
                    }
                    addBadString(sb);
                    break;
                default:
                    break;
            }
        }
    }

    private static boolean isUncertain(StringCheckStateEnum state) {
        if(state != null) {
            switch(state) {
                case UncertainNoQuote:
                case UncertainLeftQuote:
                case UncertainRightQuote:
                case Uncertain:
                    return true;
                default:
                    break;
            }
        }
        return false;
    }

    private static void checkIsAddValidate(StringBuilder sb, CharSequence str) {
        IdentityHashMap<CharSequence, StringCheckStateEnum> map = inputValidateMap.get();
        StringCheckStateEnum state = null;
        if(map != null) {
            state = map.get(sb);
        }

        if(StringCheckStateEnum.Invalidate.equals(state)) {
            return;
        }

        StringCheckStateEnum validateResult = isValidatePart(str);
        if(StringCheckStateEnum.Invalidate.equals(validateResult)) {
            addBadString(sb);
            return;
        }

        if(isUncertain(state)){
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
        addValidateString(str);
        checkIsAddValidate(sb, str);
        sb.append(v);
        return sb;
    }

    private static void checkBuilder(StringBuilder sb, String strResult) {
        IdentityHashMap<CharSequence, StringCheckStateEnum> map = inputValidateMap.get();
        StringCheckStateEnum state = null;
        if(map != null) {
            state = map.get(sb);
        }

        if(StringCheckStateEnum.Invalidate.equals(state)) {
            addBadString(strResult);
            return;
        }

        if(isUncertain(state)) {
            map.put(strResult, state);
            return;
        }

        addValidateString(strResult);
    }

    public static String handleBuilderToString(StringBuilder sb) {
        String strResult = sb.toString();
        checkBuilder(sb, strResult);
        return strResult;
    }

    public static void addBadString(CharSequence str) {
        IdentityHashMap<CharSequence, StringCheckStateEnum> map = inputValidateMap.get();
        if(map != null && str != null) {
            map.put(str, StringCheckStateEnum.Invalidate);
        }
    }

    private static void addValidateString(CharSequence str) {
        IdentityHashMap<CharSequence, StringCheckStateEnum> map = inputValidateMap.get();
        if(map != null && str != null) {
            map.put(str, StringCheckStateEnum.Validate);
        }
    }

    public static boolean isSqlValidate(String sql) {
        return isValidatePart(sql).equals(StringCheckStateEnum.Validate);
    }


    private static StringCheckStateEnum isValidatePart(CharSequence str) {
        if(str == null || str == "") {
            return StringCheckStateEnum.Validate;
        }

        IdentityHashMap<CharSequence, StringCheckStateEnum> map = inputValidateMap.get();
        if(map != null) {
            StringCheckStateEnum oldV = map.get(str);
            if(oldV != null) {
                switch(oldV) {
                    case Invalidate:
                        return StringCheckStateEnum.Invalidate;
                    case UncertainNoQuote:
                    case UncertainLeftQuote:
                    case UncertainRightQuote:
                    case Uncertain:
                        return StringCheckStateEnum.Uncertain;
                    case Validate:
                        return StringCheckStateEnum.Validate;
                    default:
                        break;
                }
            }
        }

        if(isNumberic(str)) {
            return StringCheckStateEnum.Validate;
        }

        if(isSafeValue(str)) {
            map.put(str, StringCheckStateEnum.UncertainNoQuote);
            return StringCheckStateEnum.Uncertain;
        }
        return StringCheckStateEnum.Invalidate;
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


    private static enum StringCheckStateEnum
    {
        Validate,
        Invalidate,
        Uncertain,
        UncertainNoQuote,
        UncertainLeftQuote,
        UncertainRightQuote
    }
}