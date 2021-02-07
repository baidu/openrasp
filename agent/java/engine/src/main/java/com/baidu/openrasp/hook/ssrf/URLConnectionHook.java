/*
 * Copyright 2017-2021 Baidu Inc.
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

import com.baidu.openrasp.hook.ssrf.redirect.AbstractRedirectHook;
import com.baidu.openrasp.hook.ssrf.redirect.URLConnectionRedirectHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;
import sun.net.www.protocol.http.HttpURLConnection;

import java.io.IOException;
import java.net.URL;
import java.net.URLConnection;
import java.util.HashMap;

/**
 * Created by tyy on 17-12-7.
 * <p>
 * jdk 中进行 http 请求的 hook 点
 */
@HookAnnotation
public class URLConnectionHook extends AbstractSSRFHook {

    private static ThreadLocal<Boolean> isChecking = new ThreadLocal<Boolean>() {
        @Override
        protected Boolean initialValue() {
            return false;
        }
    };

    private static ThreadLocal<Boolean> isExit = new ThreadLocal<Boolean>() {
        @Override
        protected Boolean initialValue() {
            return false;
        }
    };

    private static ThreadLocal<HashMap<String, Object>> originCache = new ThreadLocal<HashMap<String, Object>>() {
        @Override
        protected HashMap<String, Object> initialValue() {
            return null;
        }
    };

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "sun/net/www/protocol/http/HttpURLConnection".equals(className) ||
                "weblogic/net/http/HttpURLConnection".equals(className);
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
        insertBefore(ctClass, "getInputStream", "()Ljava/io/InputStream;", src);
        src = getInvokeStaticSrc(URLConnectionHook.class, "onExit", "$0", Object.class);
        insertAfter(ctClass, "getInputStream", "()Ljava/io/InputStream;", src, true);
    }

    public static void onExit(Object urlConnection) {
        try {
            if (isChecking.get() && !isExit.get() && URLConnectionRedirectHook.urlCache.get() != null) {
                // 以下会继续调用 getinpustream isExit 避免死循环
                isExit.set(true);
                HashMap<String, Object> cache = originCache.get();
                HashMap<String, Object> redirectCache = getSsrfParamWithURL(URLConnectionRedirectHook.urlCache.get());
                if (cache != null && redirectCache != null) {
                    AbstractRedirectHook.checkRedirect(cache, redirectCache,
                            ((HttpURLConnection) urlConnection).getResponseMessage(), ((HttpURLConnection) urlConnection).getResponseCode());
                }
            }
        } catch (Exception e) {
            // ignore
        } finally {
            isChecking.set(false);
            originCache.set(null);
            isExit.set(false);
            URLConnectionRedirectHook.urlCache.set(null);
        }
    }

    private static HashMap<String, Object> getSsrfParamWithURL(URL url) {
        try {
            String host = null;
            String port = "";
            if (url != null) {
                host = url.getHost();
                int temp = url.getPort();
                if (temp > 0) {
                    port = temp + "";
                }
            }
            if (url != null && host != null) {
                return getSsrfParam(url.toString(), host, port, "url_open_connection");
            }
        } catch (Exception e) {
            // ignore
        }
        return null;
    }

    public static void checkHttpConnection(URLConnection urlConnection) {
        if (!isChecking.get()) {
            isChecking.set(true);
            if (urlConnection != null) {
                URL url = urlConnection.getURL();
                HashMap<String, Object> param = getSsrfParamWithURL(url);
                checkHttpUrl(param);
                originCache.set(param);
            }
        }
    }
}
