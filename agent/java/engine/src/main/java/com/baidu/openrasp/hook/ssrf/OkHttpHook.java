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

import com.baidu.openrasp.messaging.LogTool;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;
import java.net.URL;

/**
 * @description: okhttp的ssrf检测hook点
 * @author: anyang
 * @create: 2018/10/09 19:40
 */
@HookAnnotation
public class OkHttpHook extends AbstractSSRFHook {
    @Override
    public boolean isClassMatched(String className) {
        // com/squareup/okhttp/Call$ApplicationInterceptorChain 类适用于 okhttp2.2 版本以上
        return "com/squareup/okhttp/Call$ApplicationInterceptorChain".equals(className) ||
                "okhttp3/RealCall$ApplicationInterceptorChain".equals(className) ||
                "okhttp3/internal/http/RealInterceptorChain".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(OkHttpHook.class, "checkOkHttpUrl",
                "$1", Object.class);
        if (ctClass.getName().contains("RealCall$ApplicationInterceptorChain")) {
            insertBefore(ctClass, "proceed",
                    "(Lokhttp3/Request;)Lokhttp3/Response;", src);
        } else if (ctClass.getName().contains("RealInterceptorChain")) {
            insertBeforeWithExclude(ctClass, "proceed",
                    "(Lokhttp3/Request;)Lokhttp3/Response;", src);
        } else {
            insertBefore(ctClass, "proceed",
                    "(Lcom/squareup/okhttp/Request;)Lcom/squareup/okhttp/Response;", src);
        }
    }

    public static void checkOkHttpUrl(Object request) {
        String host = null;
        String port = "";
        Object url = null;
        if (request != null) {
            try {
                url = Reflection.invokeMethod(request, "url", new Class[]{});
                if (url == null) {
                    return;
                }
                int portTemp;
                if (url instanceof URL) {
                    host = ((URL) url).getHost();
                    portTemp = ((URL) url).getPort();
                    if (portTemp > 0) {
                        port = portTemp + "";
                    }
                } else {
                    host = Reflection.invokeStringMethod(url, "host", new Class[]{});
                    Integer portData = (Integer) Reflection.invokeMethod(url, "port", new Class[]{});
                    if (portData != null && portData > 0) {
                        port = String.valueOf(portData);
                    }
                }
                if (host == null) {
                    return;
                }
            } catch (Throwable t) {
                LogTool.traceHookWarn("parse url " + url + " failed: " + t.getMessage(), t);
            }
        }
        if (url != null) {
            checkHttpUrl(url.toString(), host, port, "okhttp");
        }
    }
}
