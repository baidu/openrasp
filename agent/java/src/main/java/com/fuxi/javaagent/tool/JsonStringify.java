/**
 * Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

package com.fuxi.javaagent.tool;

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
