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
import java.net.URL;
import java.net.URLConnection;

/**
 * Created by tyy on 17-12-7.
 * <p>
 * jdk 中进行 http 请求的 hook 点
 */
@HookAnnotation
public class URLConnectionHook extends AbstractSSRFHook {

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
        insertBefore(ctClass, "connect", "()V", src);
    }

    public static void checkHttpConnection(URLConnection urlConnection) {
        URL url = null;
        String host = null;
        String port = "";
        try {
            if (urlConnection != null) {
                url = urlConnection.getURL();
                if (url != null) {
                    host = url.getHost();
                    int temp = url.getPort();
                    if (temp > 0) {
                        port = temp + "";
                    }
                }
            }
        } catch (Exception e) {
            String message = url != null ? ("parse url " + url + "failed") : e.getMessage();
            int errorCode = ErrorType.HOOK_ERROR.getCode();
            HookHandler.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
        }
        if (url != null && host != null) {
            checkHttpUrl(url.toString(), host, port, "url_open_connection");
        }
    }

}
