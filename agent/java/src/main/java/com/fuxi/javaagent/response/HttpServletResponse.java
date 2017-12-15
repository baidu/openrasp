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

package com.fuxi.javaagent.response;

import com.fuxi.javaagent.HookHandler;
import com.fuxi.javaagent.config.Config;
import com.fuxi.javaagent.tool.Reflection;

/**
 * Created by tyy on 9/5/17.
 * javax.servlet.http.HttpServletResponse类型响应的统一接口
 */
public class HttpServletResponse {

    public static final int BLOCK_STATUS_CODE = 400;
    public static final String CONTENT_TYPE_HEADER_KEY = "Content-Type";
    public static final String CONTENT_LENGTH_HEADER_KEY = "Content-Length";
    public static final String CONTENT_TYPE_HTML_VALUE = "text/html";

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
    public void sendError() {
        if (response != null) {
            try {
                setHeader(CONTENT_TYPE_HEADER_KEY, CONTENT_TYPE_HTML_VALUE);
                Reflection.invokeMethod(response, "setStatus", new Class[]{int.class}, BLOCK_STATUS_CODE);

                String blockUrl = Config.getConfig().getBlockUrl();
                if (!blockUrl.contains("?")) {
                    String blockParam = "?request_id=" + HookHandler.requestCache.get().getRequestId();
                    blockUrl += blockParam;
                }
                String script = "</script><script>location.href=\"" + blockUrl + "\"</script>";

                resetBuffer();
                setIntHeader(CONTENT_LENGTH_HEADER_KEY, script.getBytes().length);
                sendContent(script, true);
            } catch (Exception e) {
                //ignore
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

}
