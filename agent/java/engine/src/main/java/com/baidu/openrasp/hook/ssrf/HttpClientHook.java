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

package com.baidu.openrasp.hook.ssrf;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;
import java.net.URI;

/**
 * Created by tyy on 17-12-8.
 * <p>
 * httpclient 框架的请求 http 的 hook 点
 */
@HookAnnotation
public class HttpClientHook extends AbstractSSRFHook {

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/http/client/methods/HttpRequestBase".equals(className);
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(HttpClientHook.class, "checkHttpUri",
                "$1", URI.class);
        insertBefore(ctClass, "setURI", "(Ljava/net/URI;)V", src);
    }

    public static void checkHttpUri(URI uri) {
        String url = null;
        String hostName = null;
        String port = "";
        try {
            if (uri != null) {
                url = uri.toString();
                hostName = uri.toURL().getHost();
                int temp = uri.toURL().getPort();
                if (temp > 0) {
                    port = temp + "";
                }
            }
        } catch (Throwable t) {
            String message = url != null ? ("parse url " + url + "failed") : t.getMessage();
            int errorCode = ErrorType.HOOK_ERROR.getCode();
            HookHandler.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), t);
        }
        if (hostName != null) {
            checkHttpUrl(url, hostName, port, "httpclient");
        }
    }
}
