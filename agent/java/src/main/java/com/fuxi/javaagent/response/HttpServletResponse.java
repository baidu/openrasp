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
    private static final String BLOCK_INSERT_SCRIPT = "</script><script>location.href=\"" +
            "https://rasp.baidu.com/blocked\"</script>";
    private static final String CONTENT_TYPE_HEADER_KEY = "Content-Type";
    private static final String CONTENT_TYPE_HTML_VALUE = "text/html";
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
     * 设置响应头
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
     * 返回异常信息
     */
    public void sendError() {
        if (response != null) {
            try {
                setHeader(CONTENT_TYPE_HEADER_KEY, CONTENT_TYPE_HTML_VALUE);
                Reflection.invokeMethod(response, "setStatus", new Class[]{int.class}, BLOCK_STATUS_CODE);
            } catch (Exception e) {
                //ignore
            }
            sendErrorScript(response);
        }
    }

    /**
     * 发送自定义错误处理脚本
     */
    private void sendErrorScript(Object response) {
        String blockUrl = Config.getConfig().getBlockUrl();
        if (!blockUrl.contains("?")) {
            String blockParam = "?request_id=" + HookHandler.requestCache.get().getRequestId();
            blockUrl += blockParam;
        }
        String script = "</script><script>location.href=\"" + blockUrl + "\"</script>";
        Object printer = Reflection.invokeMethod(response, "getWriter", new Class[]{});
        Reflection.invokeMethod(printer, "print", new Class[]{String.class}, script);
        Reflection.invokeMethod(printer, "flush", new Class[]{});
        Reflection.invokeMethod(response, "flushBuffer", new Class[]{});
        Reflection.invokeMethod(printer, "close", new Class[]{});
    }

}
