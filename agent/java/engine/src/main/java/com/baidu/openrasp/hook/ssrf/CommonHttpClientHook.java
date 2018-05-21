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
import com.baidu.openrasp.tool.Reflection;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * Created by tyy on 17-12-7.
 *
 * commons-httpclinet 框架的 http 请求 hook 点
 */
public class CommonHttpClientHook extends AbstractSSRFHook {

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/commons/httpclient/HttpClient".equals(className);
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(CommonHttpClientHook.class, "checkHttpConnection",
                "$2", Object.class);
        insertBefore(ctClass, "executeMethod",
                "(Lorg/apache/commons/httpclient/HostConfiguration;" +
                        "Lorg/apache/commons/httpclient/HttpMethod;" +
                        "Lorg/apache/commons/httpclient/HttpState;)I", src);
    }

    public static void checkHttpConnection(Object httpMethod) {
        String host = null;
        Object uri = null;
        try {
            if (httpMethod != null) {
                uri = Reflection.invokeMethod(httpMethod, "getURI", new Class[]{});
                if (uri != null) {
                    host = Reflection.invokeStringMethod(uri, "getHost", new Class[]{});
                }
            }
        } catch (Throwable t) {
            HookHandler.LOGGER.warn(t.getMessage());
        }
        if (host != null) {
            checkHttpUrl(uri.toString(), host, "commons_httpclient");
        }
    }
}
