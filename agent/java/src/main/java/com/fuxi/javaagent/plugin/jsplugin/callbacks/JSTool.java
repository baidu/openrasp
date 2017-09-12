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

package com.fuxi.javaagent.plugin.jsplugin.callbacks;

import com.eclipsesource.v8.*;
import com.fuxi.javaagent.tool.StackTrace;

/**
 * Created by tyy on 9/12/17.
 * 提供给js插件的java工具类
 */
public class JSTool extends V8Object {

    public static final int V8_STACK_TRACE_MAX_DEPTH = 100;

    public JSTool(final V8 v8) {
        super(v8);
        V8Function getStackTrace = new V8Function(v8, new JavaCallback() {
            @Override
            public Object invoke(V8Object receiver, V8Array parameters) {
                V8Array stackTrace = new V8Array(v8);
                StackTraceElement[] stackTraceElements = StackTrace.getStackTraceArray();
                if (stackTraceElements != null) {
                    int startIndex = 12;
                    for (int i = startIndex; i < stackTraceElements.length; i++) {
                        if (i > V8_STACK_TRACE_MAX_DEPTH + startIndex) {
                            break;
                        }
                        stackTrace.push(stackTraceElements[i].getClassName() + "@" + stackTraceElements[i].getMethodName());
                    }
                }
                return stackTrace;
            }
        });
        this.add("getStackTrace", getStackTrace);
        getStackTrace.release();

    }

}
