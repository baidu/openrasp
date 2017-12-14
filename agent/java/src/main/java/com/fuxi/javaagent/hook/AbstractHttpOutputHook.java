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

package com.fuxi.javaagent.hook;

import com.fuxi.javaagent.HookHandler;
import com.fuxi.javaagent.config.Config;
import com.fuxi.javaagent.response.HttpServletResponse;
import com.fuxi.javaagent.tool.Reflection;
import org.apache.commons.lang3.StringUtils;

/**
 * Created by tyy on 17-12-13.
 *
 * http 输出 hook 点
 */
public abstract class AbstractHttpOutputHook extends AbstractClassHook {

    public static final int CATALINA_OUTPUT = 0;
    public static final int JETTY_OUTPUT = 1;

    @Override
    public String getType() {
        return "http_output";
    }

    /**
     * 向响应的 html 页面插入自定义 js 脚本
     *
     * @param output 输出流
     */
    public static void appendResponseData(Object output, int outputType) {
        boolean enableHookCache = HookHandler.isEnableCurrThreadHook();
        try {
            HookHandler.disableCurrThreadHook();
            Boolean isClosed = (Boolean) Reflection.invokeMethod(output, "isClosed", new Class[]{});
            if (isClosed != null && !isClosed) {
                HttpServletResponse response = HookHandler.responseCache.get();
                String contentType = null;
                if (response != null) {
                    contentType = response.getContentType();
                }
                if (contentType != null && contentType.contains(HttpServletResponse.CONTENT_TYPE_HTML_VALUE)) {
                    String appendScript = Config.getConfig().getCustomResponseScript();
                    if (!StringUtils.isEmpty(appendScript)) {
                        String outputMethod = null;
                        if (outputType == CATALINA_OUTPUT) {
                            outputMethod = "write";
                        } else if (outputType == JETTY_OUTPUT) {
                            outputMethod = "print";
                        }
                        if (outputMethod != null) {
                            Reflection.invokeMethod(output, outputMethod, new Class[]{String.class},
                                    "</script><script>" + appendScript + "</script>");
                        }
                    }
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            if (enableHookCache) {
                HookHandler.enableCurrThreadHook();
            }
        }
    }

}
