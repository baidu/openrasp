/**
 * Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
 * <p>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * <p>
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * <p>
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * <p>
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * <p>
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

package com.fuxi.javaagent.plugin;

import com.fuxi.javaagent.request.AbstractRequest;
import org.mozilla.javascript.*;
import org.mozilla.javascript.typedarrays.NativeUint8Array;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.Enumeration;
import java.util.Map;

public class JSRequestContext extends ScriptableObject {
    private AbstractRequest javaContext = null;
    private static Object[] properties = new Object[]{
            "path",
            "method",
            "url",
            "querystring",
            "protocol",
            "body",
            "header",
            "parameter",
            "remoteAddr",
            "server"
    };

    public JSRequestContext() {
    }

    public void jsConstructor(Object javaContext) {
        this.javaContext = (AbstractRequest) javaContext;
    }

    @Override
    public String getClassName() {
        return "Context";
    }


    public String jsGet_path() {
        return javaContext.getRequestURI();
    }

    public String jsGet_method() {
        return javaContext.getMethod().toLowerCase();
    }

    public String jsGet_url() {
        StringBuffer requestURL = javaContext.getRequestURL();
        return requestURL.toString();
    }

    public String jsGet_querystring() {
        return javaContext.getQueryString();
    }

    public String jsGet_protocol() {
        return javaContext.getProtocol().toLowerCase();
    }

    public Object jsGet_body() {
        ByteArrayOutputStream body = javaContext.getBodyStream();
        if (body == null) {
            return Context.getUndefinedValue();
        }
        JSContext cx = (JSContext) JSContext.getCurrentContext();
        final Scriptable buffer = cx.newObject(cx.scope, "Uint8Array");
        try {
            body.writeTo(new OutputStream() {
                int count = 0;

                @Override
                public void write(int b) throws IOException {
                    buffer.put(count++, buffer, b);
                }
            });
        } catch (Exception e) {
            return e;
        }
        return buffer;
    }

    public Object jsGet_header() {
        JSContext cx = (JSContext) JSContext.getCurrentContext();
        Scriptable header = cx.newObject(cx.scope);
        Enumeration<String> headerNames = javaContext.getHeaderNames();
        while (headerNames.hasMoreElements()) {
            String key = headerNames.nextElement();
            String value = javaContext.getHeader(key);
            header.put(key.toLowerCase(), header, value);
        }
        return header;
    }

    public Object jsGet_parameter() {
        JSContext cx = (JSContext) JSContext.getCurrentContext();
        Scriptable parameter = cx.newObject(cx.scope);
        Map<String, String[]> parameterMap = javaContext.getParameterMap();
        for (Map.Entry<String, String[]> entry : parameterMap.entrySet()) {
            String key = entry.getKey();
            String[] value = entry.getValue();
            int length = value.length;
            Scriptable arr = cx.newArray(cx.scope, length);
            for (int i = 0; i < length; i++) {
                arr.put(i, arr, value[i]);
            }
            parameter.put(key, parameter, arr);
        }
        return parameter;
    }

    public String jsGet_remoteAddr() {
        return javaContext.getRemoteAddr();
    }

    public Object jsGet_server() {
        JSContext cx = (JSContext) JSContext.getCurrentContext();
        Scriptable server = cx.newObject(cx.scope);
        Map<String, String> serverContext = javaContext.getServerContext();
        for (Map.Entry<String, String> entry : serverContext.entrySet()) {
            String key = entry.getKey();
            String value = entry.getValue();
            server.put(key, server, value);
        }
        return server;
    }

    @Override
    public Object[] getIds() {
        return properties;
    }
}