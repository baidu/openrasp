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

package com.baidu.openrasp.tool.decomplie;


import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

/**
 * @description: 过滤堆栈查找用户代码
 * @author: anyang
 * @create: 2018/10/21 15:07
 */
public class StackTraceFilter {
    public Map<String, String> class_method = new HashMap<String, String>();
    public Map<String, Integer> class_lineNumber = new HashMap<String, Integer>();
    private static Set<String> fiterSet = new HashSet<String>();

    static {
        fiterSet.add("com.alibaba");
        fiterSet.add("org.");
        fiterSet.add("java.");
        fiterSet.add("javax.");
        fiterSet.add("jdk.");
        fiterSet.add("sun.");
        fiterSet.add("apache.");
        fiterSet.add("com.caucho");
        fiterSet.add("com.sun");
        fiterSet.add("com.google");
        fiterSet.add("ognl.");
        fiterSet.add("com.mysql");
        fiterSet.add("com.microsoft");
        fiterSet.add("oracle.");
        fiterSet.add("com.ibm");
        fiterSet.add("okhttp3.");
        fiterSet.add("com.squareup");
        fiterSet.add("weblogic");
        fiterSet.add("net.");
    }

    public void filter(StackTraceElement[] trace) {
        for (StackTraceElement element : trace) {
            boolean isMatched = false;
            for (String filterString : fiterSet) {
                if (element.getClassName().startsWith(filterString)) {
                    isMatched =true;
                    break;
                }
            }
            if (!isMatched){
                class_method.put(element.getClassName(), element.getMethodName());
                class_lineNumber.put(element.getClassName(), element.getLineNumber());
            }
        }
    }
}
