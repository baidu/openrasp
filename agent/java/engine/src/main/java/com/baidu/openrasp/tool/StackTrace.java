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

import java.util.LinkedList;
import java.util.List;

/**
 * Created by tyy on 3/28/17.
 * 获取栈信息工具类
 */
public class StackTrace {

    /**
     * 获取栈信息
     *
     * @return 栈信息
     */
    public static String getStackTrace() {

        Throwable throwable = new Throwable();
        StackTraceElement[] stackTraceElements = throwable.getStackTrace();
        StringBuilder retStack = new StringBuilder();

        //此处前几个调用栈都是插件中产生的所以跳过，只显示客户自己的调用栈
        if (stackTraceElements.length >= 3) {
            for (int i = 2; i < stackTraceElements.length; i++) {
                retStack.append(stackTraceElements[i].getClassName() + "@" + stackTraceElements[i].getMethodName()
                        + "(" + stackTraceElements[i].getLineNumber() + ")" + "\r\n");
            }
        } else {
            for (int i = 0; i < stackTraceElements.length; i++) {
                retStack.append(stackTraceElements[i].getClassName() + "@" + stackTraceElements[i].getMethodName()
                        + "(" + stackTraceElements[i].getLineNumber() + ")" + "\r\n");
            }
        }

        return retStack.toString();
    }

    /**
     * 获取原始栈
     *
     * @return 原始栈
     */
    public static List<String> getStackTraceArray(int startIndex, int depth) {

        LinkedList<String> stackTrace = new LinkedList<String>();
        Throwable throwable = new Throwable();
        StackTraceElement[] stackTraceElements = throwable.getStackTrace();

        if (stackTraceElements != null) {
            for (int i = startIndex; i < stackTraceElements.length; i++) {
                if (i > startIndex + depth) {
                    break;
                }
                stackTrace.add(stackTraceElements[i].getClassName() + "." + stackTraceElements[i].getMethodName());
            }
        }

        return stackTrace;
    }

}
