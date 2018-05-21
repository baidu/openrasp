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

package com.baidu.openrasp.hook.server;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.response.HttpServletResponse;
import com.baidu.openrasp.tool.Reflection;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;
import org.apache.commons.lang3.StringUtils;

import java.io.IOException;

/**
 * Created by tyy on 17-12-13.
 *
 * 服务器 http 输出关闭 hook 点基类
 * 用于向输出流中插入自定义内容
 */
public abstract class ServerOutputCloseHook extends AbstractClassHook {

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    public String getType() {
        return "http_output";
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(ServerOutputCloseHook.class, "appendResponseData",
                "$0", Object.class);
        hookMethod(ctClass, src);
    }

    /**
     * hook 方法
     *
     * @param ctClass hook 点所在的类
     * @param src     加入 hook点的代码
     */
    protected abstract void hookMethod(CtClass ctClass, String src) throws NotFoundException, CannotCompileException;

    /**
     * 向响应的 html 页面插入自定义 js 脚本
     *
     * @param output 输出流
     */
    public static void appendResponseData(Object output) {
        if (HookHandler.enableHook.get() && HookHandler.isEnableCurrThreadHook()) {
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
                        String injectPathPrefix = Config.getConfig().getInjectUrlPrefix();
                        if (!StringUtils.isEmpty(injectPathPrefix)) {
                            if (HookHandler.requestCache.get().getRequestURL().toString().startsWith(injectPathPrefix)) {
                                String appendHtml = Config.getConfig().getCustomResponseScript();
                                if (!StringUtils.isEmpty(appendHtml)) {
                                    response.sendContent(appendHtml, false);
                                }
                            }
                        }
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
            } finally {
                HookHandler.enableCurrThreadHook();
            }
        }
    }

}
