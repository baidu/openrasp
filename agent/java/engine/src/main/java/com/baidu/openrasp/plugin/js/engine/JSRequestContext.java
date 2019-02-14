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

package com.baidu.openrasp.plugin.js.engine;


import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.request.AbstractRequest;
import com.baidu.openrasp.request.EmptyRequest;
import com.baidu.openrasp.request.HttpServletRequest;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;
import org.mozilla.javascript.*;
import org.mozilla.javascript.annotations.JSConstructor;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.*;

import static com.baidu.openrasp.cloud.utils.CloudUtils.getMapGsonObject;

public class JSRequestContext extends ScriptableObject {
    private static final String CONTENT_TYPE_JSON_VALUE = "application/json";

    private static Set<Object> properties = new HashSet<Object>(Arrays.asList(
            "path",
            "method",
            "url",
            "querystring",
            "protocol",
            "body",
            "json",
            "header",
            "parameter",
            "remoteAddr",
            "server",
            "appBasePath"
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
        String path = javaContext.getRequestURI();
        return path == null ? "" : path;
    }

    public String jsGet_method() {
        String method = javaContext.getMethod();
        return method == null ? "" : method.toLowerCase();
    }

    public String jsGet_url() {
        StringBuffer requestURL = javaContext.getRequestURL();
        return requestURL == null ? "" : requestURL.toString();
    }

    public String jsGet_querystring() {
        String query = javaContext.getQueryString();
        return query == null ? "" : query;
    }

    public String jsGet_appBasePath() {
        String appBasePath = javaContext.getAppBasePath();
        return appBasePath == null ? "" : appBasePath;
    }

    public String jsGet_protocol() {
        String proto = javaContext.getProtocol();
        return proto == null ? "" : proto.toLowerCase();
    }

    public Object jsGet_body() {
        ByteArrayOutputStream body = javaContext.getBodyStream();
        if (body == null) {
            return cx.newObject(scope);
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
            String message = "js failed to get body";
            int errorCode = ErrorType.PLUGIN_ERROR.getCode();
            HookHandler.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
            return cx.newObject(scope);
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
        String remoteAddr = javaContext.getRemoteAddr();
        return remoteAddr == null ? "" : remoteAddr;
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

    public Object jsGet_json() {
        Scriptable json = cx.newObject(scope);
        byte[] body = javaContext.getBody();
        if (body != null) {
            String contentType = javaContext.getContentType();
            if (contentType != null && contentType.startsWith(CONTENT_TYPE_JSON_VALUE)) {
                try {
                    JsonObject jsonObject = new JsonParser().parse(new String(body)).getAsJsonObject();
                    Map<String, Object> map = getMapGsonObject().fromJson(jsonObject, Map.class);
                    for (Map.Entry<String, Object> entry : map.entrySet()) {
                        String key = entry.getKey();
                        Object value = entry.getValue();
                        json.put(key, json, value);
                    }
                } catch (Exception e) {
                    String message = "failed to parse body to json";
                    int errorCode = ErrorType.PLUGIN_ERROR.getCode();
                    HookHandler.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
                }
            }
        }
        return json;
    }
}