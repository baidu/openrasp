/*
 * Copyright 2017-2020 Baidu Inc.
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

package com.baidu.openrasp.response;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.hook.server.bes.BESResponseBodyHook;
import com.baidu.openrasp.hook.server.catalina.CatalinaResponseBodyHook;
import com.baidu.openrasp.messaging.LogTool;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.model.ApplicationModel;

/**
 * Created by tyy on 9/5/17.
 * javax.servlet.http.HttpServletResponse类型响应的统一接口
 */
public class HttpServletResponse {

    private static final int REDIRECT_STATUS_CODE = 302;
    public static final String CONTENT_TYPE_HEADER_KEY = "Content-Type";
    public static final String CONTENT_LENGTH_HEADER_KEY = "Content-Length";
    public static final String CONTENT_TYPE_REPLACE_REQUEST_ID = "%request_id%";
    public static final String CONTENT_TYPE_HTML_VALUE = "text/html";
    public static final String CONTENT_TYPE_JSON_VALUE = "application/json";
    public static final String CONTENT_TYPE_XML_VALUE = "application/xml";
    public static final String CONTENT_TYPE_TEXT_XML = "text/xml";
    private Object response;

    /**
     * constructor
     *
     * @param response http响应实体
     */
    public HttpServletResponse(Object response) {
        this.response = response;
    }

    /**
     * 获取http相应实体
     *
     * @return http响应实体
     */
    public Object getResponse() {
        return response;
    }

    /**
     * 设置响应头,覆盖原值
     *
     * @param key   响应头名称
     * @param value 响应头值
     */
    public void setHeader(String key, String value) {
        if (response != null) {
            Reflection.invokeMethod(response, "setHeader", new Class[]{String.class, String.class}, key, value);
        }
    }

    /**
     * 设置数字响应头,覆盖原值
     *
     * @param key   响应头名称
     * @param value 响应头值
     */
    public void setIntHeader(String key, int value) {
        if (response != null) {
            Reflection.invokeMethod(response, "setIntHeader", new Class[]{String.class, int.class}, key, value);
        }
    }

    /**
     * 设置响应头，不覆盖
     *
     * @param key   响应头名称
     * @param value 响应头值
     */
    public void addHeader(String key, String value) {
        if (response != null) {
            Reflection.invokeMethod(response, "addHeader", new Class[]{String.class, String.class}, key, value);
        }
    }

    /**
     * 获取响应头
     *
     * @param key 响应头名称
     * @return 响应头值值
     */
    public String getHeader(String key) {
        if (response != null) {
            Object header = Reflection.invokeMethod(response, "getHeader", new Class[]{String.class}, key);
            if (header != null) {
                return header.toString();
            }
        }
        return null;
    }

    public String getCharacterEncoding() {
        if (response != null) {
            Object enc = Reflection.invokeMethod(response, "getCharacterEncoding", new Class[]{});
            if (enc != null) {
                return enc.toString();
            }
        }
        return null;
    }

    public String getContentType() {
        if (response != null) {
            Object contentType = Reflection.invokeMethod(response, "getContentType", new Class[]{});
            if (contentType != null) {
                return contentType.toString();
            }
        }
        return null;
    }

    /**
     * 清除所有 body buffer 缓存
     *
     * @return 是否成功
     */
    public boolean resetBuffer() {
        if (response != null) {
            try {
                Reflection.invokeMethod(response, "resetBuffer", new Class[]{});
            } catch (Exception e) {
                return false;
            }
            return true;
        }
        return false;
    }

    /**
     * 清除所有 buffer 缓存
     *
     * @return 是否成功
     */
    public boolean reset() {
        if (response != null) {
            try {
                Reflection.invokeMethod(response, "reset", new Class[]{});
            } catch (Exception e) {
                return false;
            }
            return true;
        }
        return false;
    }

    /**
     * 返回异常信息
     */
    public void sendError(CheckParameter parameter) {
        if (response != null) {
            try {
                int statusCode = Config.getConfig().getBlockStatusCode();
                String blockUrl = Config.getConfig().getBlockUrl();
                boolean isCommitted = (Boolean) Reflection.invokeMethod(response, "isCommitted", new Class[]{});
                String requestId = HookHandler.requestCache.get().getRequestId();
                String contentType = getResponseContentType();
                String script;
                if (contentType.startsWith(CONTENT_TYPE_JSON_VALUE)) {
                    script = Config.getConfig().getBlockJson().replace(CONTENT_TYPE_REPLACE_REQUEST_ID, requestId);
                } else if (contentType.startsWith(CONTENT_TYPE_XML_VALUE) || contentType.startsWith(CONTENT_TYPE_TEXT_XML)) {
                    script = Config.getConfig().getBlockXml().replace(CONTENT_TYPE_REPLACE_REQUEST_ID, requestId);
                } else {
                    script = Config.getConfig().getBlockHtml().replace(CONTENT_TYPE_REPLACE_REQUEST_ID, requestId);
                }
                if (parameter.getType().equals(CheckParameter.Type.XSS_USERINPUT)) {
                    if ("tomcat".equals(ApplicationModel.getServerName())) {
                        CatalinaResponseBodyHook.handleXssBlockBuffer(parameter, script);
                    } else if ("bes".equals(ApplicationModel.getServerName())) {
                        BESResponseBodyHook.handleXssBlockBuffer(parameter, script);
                    }
                }else {
                    if (!isCommitted) {
                        resetBuffer();
                        Reflection.invokeMethod(response, "setStatus", new Class[]{int.class}, statusCode);
                        if (statusCode >= 300 && statusCode <= 399) {
                            setHeader("Location", blockUrl.replace(CONTENT_TYPE_REPLACE_REQUEST_ID, requestId));
                        }
                        setIntHeader(CONTENT_LENGTH_HEADER_KEY, script.getBytes().length);
                    }
                    sendContent(script, true);
                }
            } catch (Exception e) {
                LogTool.traceHookWarn("failed to handle block body: " + e.getMessage(), e);
            }
        }
    }

    /**
     * 发送自定义错误处理脚本
     */
    public void sendContent(String content, boolean close) {
        Object printer = null;

        printer = Reflection.invokeMethod(response, "getWriter", new Class[]{});
        if (printer == null) {
            printer = Reflection.invokeMethod(response, "getOutputStream", new Class[]{});
        }
        Reflection.invokeMethod(printer, "print", new Class[]{String.class}, content);
        Reflection.invokeMethod(printer, "flush", new Class[]{});
        if (close) {
            Reflection.invokeMethod(printer, "close", new Class[]{});
        }
    }

    /**
     * 获取响应发送脚本的ContentType类型
     */
    public String getResponseContentType() {
        String contentType = getContentType();
        if (contentType == null) {
            contentType = HookHandler.requestCache.get().getHeader("Accept");
            if (!contentType.startsWith(CONTENT_TYPE_JSON_VALUE) && !contentType.startsWith(CONTENT_TYPE_XML_VALUE) && !contentType.startsWith(CONTENT_TYPE_TEXT_XML)) {
                contentType = CONTENT_TYPE_HTML_VALUE;
            }
        }
        return contentType;
    }

}
