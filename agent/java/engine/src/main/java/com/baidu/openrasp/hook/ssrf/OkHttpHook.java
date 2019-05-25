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
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * @description: okhttp的ssrf检测hook点
 * @author: anyang
 * @create: 2018/10/09 19:40
 */
@HookAnnotation
public class OkHttpHook extends AbstractSSRFHook {
    @Override
    public boolean isClassMatched(String className) {
        return "okhttp3/HttpUrl".equals(className) ||
                "com/squareup/okhttp/HttpUrl".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(OkHttpHook.class, "checkOkHttpUrl",
                "$1,$_", String.class, Object.class);
        insertAfter(ctClass, "parse", "(Ljava/lang/String;)Lokhttp3/HttpUrl;", src);
        insertAfter(ctClass, "parse", "(Ljava/lang/String;)Lcom/squareup/okhttp/HttpUrl;", src);
    }

    public static void checkOkHttpUrl(String url, Object httpUrl) {
        String host = null;
        String port = "";
        if (httpUrl != null) {
            try {
                host = Reflection.invokeStringMethod(httpUrl, "host", new Class[]{});
                Integer object = (Integer)Reflection.invokeMethod(httpUrl, "port", new Class[]{});
                if (object != null && object > 0) {
                    port = String.valueOf(object);
                }
            } catch (Exception e) {
                String message = url != null ? ("parse url " + url + "failed") : e.getMessage();
                int errorCode = ErrorType.HOOK_ERROR.getCode();
                HookHandler.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
            }
        }
        if (host != null) {
            checkHttpUrl(url, host, port, "okhttp");
        }
    }
}
