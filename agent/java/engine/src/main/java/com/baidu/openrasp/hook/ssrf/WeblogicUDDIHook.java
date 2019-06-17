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

/**
 * @description: weblogic UDDI
 * @author: anyang
 * @create: 2019/01/28 11:21
 */
@HookAnnotation
public class WeblogicUDDIHook extends AbstractSSRFHook {

    @Override
    public boolean isClassMatched(String className) {
        return "weblogic/uddi/client/service/UDDIService".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(WeblogicUDDIHook.class, "getURL", "$1", String.class);
        insertBefore(ctClass, "setURL", null, src);
    }

    public static void getURL(String weblogicURL) {
        if (weblogicURL != null) {
            URL url = null;
            String host = null;
            String port = "";
            try {
                url = new URL(weblogicURL);
                host = url.getHost();
                int temp = url.getPort();
                if (temp > 0) {
                    port = temp + "";
                }
            } catch (Exception e) {
                String message = url != null ? ("parse url " + url + "failed") : e.getMessage();
                int errorCode = ErrorType.HOOK_ERROR.getCode();
                HookHandler.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
            }
            if (url != null && host != null) {
                checkHttpUrl(weblogicURL, host, port, "weblogic_UDDI");
            }
        }
    }
}
