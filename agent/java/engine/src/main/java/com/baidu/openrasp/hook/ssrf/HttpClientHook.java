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
import com.baidu.openrasp.hook.ssrf.redirect.HttpClientRedirectHook;
import com.baidu.openrasp.messaging.LogTool;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtBehavior;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;
import java.net.URI;
import java.util.HashMap;
import java.util.LinkedList;

/**
 * Created by tyy on 17-12-8.
 * <p>
 * httpclient 框架的请求 http 的 hook 点
 */
@HookAnnotation
public class HttpClientHook extends AbstractSSRFHook {

    private static ThreadLocal<Boolean> isChecking = new ThreadLocal<Boolean>() {
        @Override
        protected Boolean initialValue() {
            return false;
        }
    };

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/http/impl/client/CloseableHttpClient".equals(className)
                || "org/apache/http/impl/client/AutoRetryHttpClient".equals(className)
                || "org/apache/http/impl/client/DecompressingHttpClient".equals(className)
                // 兼容 4.0 版本, 4.0 版本没有 CloseableHttpClient
                || "org/apache/http/impl/client/AbstractHttpClient".equals(className);
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        CtClass[] interfaces = ctClass.getInterfaces();
        if (interfaces != null) {
            for (CtClass inter : interfaces) {
                // 兼容 http client 4.0 版本的 AbstractHttpClient
                if (inter.getName().equals("org.apache.http.client.HttpClient")) {
                    LinkedList<CtBehavior> methods =
                            getMethod(ctClass, "execute", null, null);
                    String afterSrc = getInvokeStaticSrc(HttpClientHook.class, "exitCheck",
                            "$1,$_", Object.class, Object.class);
                    for (CtBehavior method : methods) {
                        if (method.getSignature().startsWith("(Lorg/apache/http/client/methods/HttpUriRequest")) {
                            String src = getInvokeStaticSrc(HttpClientHook.class,
                                    "checkHttpUri", "$1", Object.class);
                            insertBefore(method, src);
                            insertAfter(method, afterSrc, true);
                        } else if (method.getSignature().startsWith("(Lorg/apache/http/HttpHost")) {
                            String src = getInvokeStaticSrc(HttpClientHook.class,
                                    "checkHttpHost", "$1", Object.class);
                            insertBefore(method, src);
                            insertAfter(method, afterSrc, true);
                        }
                    }
                    break;
                }
            }
        }
    }

    public static void exitCheck(Object uriValue, Object response) {
        try {
            if (isChecking.get() && response != null) {

                URI redirectUri = HttpClientRedirectHook.uriCache.get();
                if (redirectUri != null) {
                    HashMap<String, Object> params = getSsrfParam(uriValue);
                    if (params != null) {
                        HashMap<String, Object> redirectParams = getSsrfParamFromURI(redirectUri);
                        if (redirectParams != null) {
                            AbstractRedirectHook.checkHttpClientRedirect(params, redirectParams, response);
                        }
                    }
                }
            }
        } finally {
            isChecking.set(false);
            HttpClientRedirectHook.uriCache.set(null);
        }
    }

    private static HashMap<String, Object> getSsrfParam(Object value) {
        if (value.getClass().getName().contains("HttpHost")) {
            return getSsrfParamFromHostValue(value);
        } else {
            URI uri = (URI) Reflection.invokeMethod(value, "getURI", new Class[]{});
            return getSsrfParamFromURI(uri);
        }
    }

    private static HashMap<String, Object> getSsrfParamFromURI(URI uri) {
        if (uri != null) {
            String url = null;
            String hostName = null;
            String port = "";
            try {
                url = uri.toString();
                hostName = uri.toURL().getHost();
                int temp = uri.toURL().getPort();
                if (temp > 0) {
                    port = temp + "";
                }

            } catch (Throwable t) {
                LogTool.traceHookWarn("parse url " + url + " failed: " + t.getMessage(), t);
            }
            if (hostName != null) {
                return getSsrfParam(url, hostName, port, "httpclient");
            }
        }
        return null;
    }

    private static HashMap<String, Object> getSsrfParamFromHostValue(Object host) {
        try {
            String hostname = Reflection.invokeStringMethod(host, "getHostName", new Class[]{});
            String port = "";
            Integer portValue = (Integer) Reflection.invokeMethod(host, "getPort", new Class[]{});
            if (portValue != null && portValue > 0) {
                port = portValue.toString();
            }
            if (hostname != null) {
                return getSsrfParam(host.toString(), hostname, port, "httpclient");
            }
        } catch (Exception e) {
            return null;
        }
        return null;
    }

    public static void checkHttpHost(Object host) {
        if (!isChecking.get() && host != null) {
            isChecking.set(true);
            checkHttpUrl(getSsrfParamFromHostValue(host));
        }
    }

    public static void checkHttpUri(Object uriValue) {
        if (!isChecking.get() && uriValue != null) {
            isChecking.set(true);
            URI uri = (URI) Reflection.invokeMethod(uriValue, "getURI", new Class[]{});
            checkHttpUrl(getSsrfParamFromURI(uri));
        }
    }

}
