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

package com.baidu.openrasp.hook.ssrf;

import com.baidu.openrasp.HookHandler;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;
import java.net.URI;

/**
 * Created by tyy on 17-12-8.
 *
 * httpclient 框架的请求 http 的 hook 点
 */
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
        try {
            if (uri != null) {
                url = uri.toString();
                hostName = uri.toURL().getHost();
            }
        } catch (Throwable t) {
            HookHandler.LOGGER.warn(t.getMessage());
        }
        if (hostName != null) {
            checkHttpUrl(url, hostName, "httpclient");
        }
    }
}
