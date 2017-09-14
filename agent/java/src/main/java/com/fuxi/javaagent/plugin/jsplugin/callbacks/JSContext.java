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
import com.eclipsesource.v8.utils.MemoryManager;
import com.eclipsesource.v8.utils.V8ObjectUtils;
import com.fuxi.javaagent.request.AbstractRequest;
import com.fuxi.javaagent.tool.StackTrace;

import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Enumeration;
import java.util.Map;

/**
 * Created by lanyuhang on 2017/7/17.
 *
 * JavaScript环境中请求上下文对象
 *
 * 通过回调在Java环境中获取对应属性
 */
public class JSContext extends V8Object {
    private AbstractRequest javaContext;

    public JSContext(final V8 v8) {
        super(v8);
        final MemoryManager scope = new MemoryManager(v8);
        V8Object jsContext = new V8Object(v8);

        V8Function getMethod = new V8Function(v8, new JavaCallback() {
            @Override
            public Object invoke(V8Object receiver, V8Array parameters) {
                return javaContext.getMethod().toLowerCase();
            }
        });
        jsContext.add("getMethod", getMethod);

        V8Function getUrl = new V8Function(v8, new JavaCallback() {
            @Override
            public Object invoke(V8Object receiver, V8Array parameters) {
                StringBuffer requestURL = javaContext.getRequestURL();
                return requestURL.toString();
            }
        });
        jsContext.add("getUrl", getUrl);

        V8Function getPath = new V8Function(v8, new JavaCallback() {
            @Override
            public Object invoke(V8Object receiver, V8Array parameters) {
                return javaContext.getRequestURI();
            }
        });
        jsContext.add("getPath", getPath);

        V8Function getQuerystring = new V8Function(v8, new JavaCallback() {
            @Override
            public Object invoke(V8Object receiver, V8Array parameters) {
                return javaContext.getQueryString();
            }
        });
        jsContext.add("getQuerystring", getQuerystring);

        V8Function getProtocol = new V8Function(v8, new JavaCallback() {
            @Override
            public Object invoke(V8Object receiver, V8Array parameters) {
                return javaContext.getProtocol().toLowerCase();
            }
        });
        jsContext.add("getProtocol", getProtocol);

        V8Function getBody = new V8Function(v8, new JavaCallback() {
            @Override
            public Object invoke(V8Object receiver, V8Array parameters) {
                byte[] body = javaContext.getBody();
                if (body == null) {
                    return null;
                }
                ByteBuffer buffer = ByteBuffer.allocateDirect(body.length);
                buffer.put(body);
                return new V8ArrayBuffer(v8, buffer);
            }
        });
        jsContext.add("getBody", getBody);

        V8Function getHeader = new V8Function(v8, new JavaCallback() {
            @Override
            public Object invoke(V8Object receiver, V8Array parameters) {
                V8Object header = new V8Object(v8);
                Enumeration<String> headerNames = javaContext.getHeaderNames();
                while (headerNames.hasMoreElements()) {
                    String key = headerNames.nextElement();
                    String value = javaContext.getHeader(key);
                    header.add(key.toLowerCase(), value);
                }
                return header;
            }
        });
        jsContext.add("getHeader", getHeader);

        V8Function getParameter = new V8Function(v8, new JavaCallback() {
            @Override
            public Object invoke(V8Object receiver, V8Array parameters) {
                V8Object parameter = new V8Object(v8);
                MemoryManager scope = new MemoryManager(v8);
                Map<String, String[]> parameterMap = javaContext.getParameterMap();
                for (Map.Entry<String, String[]> entry : parameterMap.entrySet()) {
                    String key = entry.getKey();
                    V8Array value = V8ObjectUtils.toV8Array(v8, Arrays.asList(entry.getValue()));
                    parameter.add(key, value);
                }
                scope.release();
                return parameter;
            }
        });
        jsContext.add("getParameter", getParameter);

        V8Function getRemoteAddr = new V8Function(v8, new JavaCallback() {
            @Override
            public Object invoke(V8Object receiver, V8Array parameters) {
                return javaContext.getRemoteAddr();
            }
        });
        jsContext.add("getRemoteAddr", getRemoteAddr);

        V8Function getServer = new V8Function(v8, new JavaCallback() {
            @Override
            public Object invoke(V8Object receiver, V8Array parameters) {
                return V8ObjectUtils.toV8Object(v8, javaContext.getServerContext());
            }
        });
        jsContext.add("getServer", getServer);
        V8Function createInstance = (V8Function) v8.executeScript("(function(data){return new Context(data)})");
        jsContext = (V8Object) createInstance.call(v8, new V8Array(v8).push(jsContext));
        this.setPrototype(jsContext);
        scope.release();
    }

    public void setJavaContext(AbstractRequest javaContext) {
        this.javaContext = javaContext;
    }

}