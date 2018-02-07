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
import java.net.URL;
import java.net.URLConnection;

/**
 * Created by tyy on 17-12-7.
 *
 * jdk 中进行 http 请求的 hook 点
 */
public class URLConnectionHook extends AbstractSSRFHook {

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "sun/net/www/protocol/http/HttpURLConnection".equals(className);
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(URLConnectionHook.class, "checkHttpConnection",
                "$0", URLConnection.class);
        insertBefore(ctClass, "connect", "()V", src);
    }

    public static void checkHttpConnection(URLConnection urlConnection) {
        URL url = null;
        try {
            if (urlConnection != null) {
                url = urlConnection.getURL();

            }
        } catch (Exception e) {
            HookHandler.LOGGER.warn(e.getMessage());
        }
        if (url != null) {
            checkHttpUrl(url.toString(), urlConnection.getURL().getHost(), "url_open_connection");
        }
    }

}
