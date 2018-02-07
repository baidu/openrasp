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

package com.baidu.openrasp.tool;

/**
 * Created by tyy on 7/6/17.
 * json字符串格式化的工具类
 */
public class JsonStringify {

    /**
     * 处理字符串中的特殊字符，进行转义处理
     * @param jsonString 待处理的字符串
     * @return 处理后的字符串
     */
    public static String stringify(String jsonString) {
        StringBuffer tmpString = new StringBuffer();
        for (int i = 0; i < jsonString.length(); i++) {
            char c = jsonString.charAt(i);
            switch (c) {
                case '\"':
                    tmpString.append("\\\"");
                    break;
                case '\\':
                    tmpString.append("\\\\");
                    break;
                case '/':
                    tmpString.append("\\/");
                    break;
                case '\b':
                    tmpString.append("\\b");
                    break;
                case '\f':
                    tmpString.append("\\f");
                    break;
                case '\n':
                    tmpString.append("\\n");
                    break;
                case '\r':
                    tmpString.append("\\r");
                    break;
                case '\t':
                    tmpString.append("\\t");
                    break;
                default:
                    tmpString.append(c);
            }
        }
        return tmpString.toString();
    }

}
