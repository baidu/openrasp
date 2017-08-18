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
 * Created by tyy on 3/28/17.
 * 获取栈信息工具类
 */
public class StackTrace {

    /**
     * 获取栈信息
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

}
