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

package com.baidu.openrasp.plugin.js.engine;


import com.baidu.openrasp.request.AbstractRequest;
import com.baidu.openrasp.request.EmptyRequest;
import com.baidu.openrasp.request.HttpServletRequest;
import org.mozilla.javascript.*;
import org.mozilla.javascript.annotations.JSConstructor;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.*;

public class JSRequestContext extends ScriptableObject {
    private static Set<Object> properties = new HashSet<Object>(Arrays.asList(
            "path",
            "method",
            "url",
            "querystring",
            "protocol",
            "body",
            "header",
            "parameter",
            "remoteAddr",
            "server",
            "appBasePath",
            "session"
    ));
    private JSContext cx = null;
    private AbstractRequest javaContext = null;
    private Scriptable scope = null;

    public JSRequestContext() {
    }

    @JSConstructor
    public JSRequestContext(Context cx, Object[] args,
                            Function ctorObj,
                            boolean inNewExpr) {
        this.cx = (JSContext) cx;
        this.scope = this.cx.getScope();
        this.javaContext = args[0] == null ? new EmptyRequest() : (AbstractRequest) args[0];
    }

    @Override
    public String getClassName() {
        return "Context";
    }

    @Override
    public Object[] getIds() {
        properties.addAll(Arrays.asList(super.getIds()));
        return properties.toArray();
    }

    @Override
    public boolean has(String name, Scriptable start) {
        return (start == this && properties.contains(name)) || super.has(name, start);
    }


    public String jsGet_path() {
        return javaContext.getRequestURI();
    }

    public String jsGet_method() {
        String method = javaContext.getMethod();
        return method == null ? null : method.toLowerCase();
    }

    public String jsGet_url() {
        StringBuffer requestURL = javaContext.getRequestURL();
        return requestURL == null ? null : requestURL.toString();
    }

    public String jsGet_querystring() {
        return javaContext.getQueryString();
    }

    public String jsGet_appBasePath() {
        return javaContext.getAppBasePath();
    }

    public String jsGet_protocol() {
        String proto = javaContext.getProtocol();
        return proto == null ? null : proto.toLowerCase();
    }

    public Object jsGet_body() {
        ByteArrayOutputStream body = javaContext.getBodyStream();
        if (body == null) {
            return Context.getUndefinedValue();
        }
        final Scriptable buffer = cx.newObject(scope, "Uint8Array");
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
        Scriptable header = cx.newObject(scope);
        Enumeration<String> headerNames = javaContext.getHeaderNames();
        if (headerNames != null) {
            while (headerNames.hasMoreElements()) {
                String key = headerNames.nextElement();
                String value = javaContext.getHeader(key);
                header.put(key.toLowerCase(), header, value);
            }
        }
        return header;
    }

    public Object jsGet_parameter() {
        Scriptable parameter = cx.newObject(scope);
        Map<String, String[]> parameterMap = javaContext.getParameterMap();
        if (parameterMap != null) {
            for (Map.Entry<String, String[]> entry : parameterMap.entrySet()) {
                String key = entry.getKey();
                String[] value = entry.getValue();
                int length = value.length;
                Scriptable arr = cx.newArray(scope, length);
                for (int i = 0; i < length; i++) {
                    arr.put(i, arr, value[i]);
                }
                parameter.put(key, parameter, arr);
            }
        }
        return parameter;
    }

    public String jsGet_remoteAddr() {
        return javaContext.getRemoteAddr();
    }

    public Object jsGet_server() {
        Scriptable server = cx.newObject(scope);
        Map<String, String> serverContext = javaContext.getServerContext();
        if (serverContext != null) {
            for (Map.Entry<String, String> entry : serverContext.entrySet()) {
                String key = entry.getKey();
                String value = entry.getValue();
                server.put(key, server, value);
            }
        }
        return server;
    }

    public Object jsGet_session() {
        if (!(javaContext instanceof HttpServletRequest)) {
            return Context.getUndefinedValue();
        }
        Scriptable session = cx.newObject(scope);
        Object getter = new BaseFunction() {
            @Override
            public Object call(Context cx, Scriptable scope, Scriptable thisObj,
                               Object[] args) {
                if (args.length < 1 || !(args[0] instanceof String)) {
                    throw Context.reportRuntimeError("Error: Invalid Arguments");
                }
                return ((HttpServletRequest) javaContext).getSessionAttribute((String) args[0]).toString();
            }

            @Override
            public Object getDefaultValue(Class<?> hint) {
                return "[Function: getSession]";
            }
        };
        Object setter = new BaseFunction() {
            @Override
            public Object call(Context cx, Scriptable scope, Scriptable thisObj,
                               Object[] args) {
                if (args.length < 2 || !(args[0] instanceof String) || !(args[1] instanceof String)) {
                    throw Context.reportRuntimeError("Error: Invalid Arguments");
                }
                ((HttpServletRequest) javaContext).setSessionAttribute((String) args[0], (String) args[1]);
                return null;
            }

            @Override
            public Object getDefaultValue(Class<?> hint) {
                return "[Function: setSession]";
            }
        };
        ScriptableObject.defineProperty(session, "getSession", getter, ScriptableObject.READONLY);
        ScriptableObject.defineProperty(session, "setSession", setter, ScriptableObject.READONLY);
        return session;
    }
}