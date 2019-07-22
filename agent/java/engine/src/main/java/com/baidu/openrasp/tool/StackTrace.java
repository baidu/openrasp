/*
 * Copyright 2017-2019 Baidu Inc.
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

import com.baidu.openrasp.config.Config;

import java.util.Arrays;
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
    public static List<String> getStackTraceArray() {
        LinkedList<String> stackTrace = new LinkedList<String>();
        Throwable throwable = new Throwable();
        StackTraceElement[] stackTraceElements = throwable.getStackTrace();
        if (stackTraceElements != null) {
            StackTraceElement[] stack = filter(stackTraceElements);
            for (int i = 0; i < stack.length; i++) {
                stackTrace.add(stack[i].getClassName() + "." + stack[i].getMethodName());
            }
        }

        return stackTrace;
    }

    //去掉包含rasp的堆栈
    private static StackTraceElement[] filter(StackTraceElement[] trace) {
        int i = 0;
        // 去除插件本身调用栈
        while (i < trace.length && (trace[i].getClassName().startsWith("com.baidu.openrasp")
                || trace[i].getClassName().contains("reflect"))) {
            i++;
        }
        return Arrays.copyOfRange(trace, i, Math.min(i + Config.getConfig().getPluginMaxStack(), trace.length));
    }

}
