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

package com.baidu.openrasp.hook.ssrf.redirect;

import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;
import java.net.URI;

/**
 * Created by tyy on 19-11-25.
 */
@HookAnnotation
public class HttpClientRedirectHook extends AbstractRedirectHook {

    public static ThreadLocal<URI> uriCache = new ThreadLocal<URI>() {
        @Override
        protected URI initialValue() {
            return null;
        }
    };

    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/http/impl/client/DefaultRedirectStrategy".equals(className)
                || "org/apache/http/impl/client/DefaultRedirectHandler".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(HttpClientRedirectHook.class, "cacheHttpRedirect",
                "$_", Object.class);
        insertAfter(ctClass, "getLocationURI", null, src);
    }

    public static void cacheHttpRedirect(Object uri) {
        if (uri instanceof URI) {
            uriCache.set((URI) uri);
        }
    }

}
