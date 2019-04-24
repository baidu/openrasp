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

package com.baidu.openrasp.hook;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;
import java.util.HashMap;

import java.io.IOException;

/**
 * Created by lxk on 10/12/17.
 * All rights reserved
 */
@HookAnnotation
public class WebDAVCopyResourceHook extends AbstractClassHook {
    /**
     * (none-javadoc)
     *
     * @see AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "webdav";
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/catalina/servlets/WebdavServlet".equals(className);
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(WebDAVCopyResourceHook.class, "checkWebdavCopyResource",
                "$0,$3,$4", Object.class, String.class, String.class);
        insertBefore(ctClass, "copyResource",
                "(Ljavax/naming/directory/DirContext;Ljava/util/Hashtable;Ljava/lang/String;Ljava/lang/String;)Z", src);
    }

    /**
     * 检测WebDAV COPY MOVE
     *
     * @param webdavServlet
     * @param source
     * @param dest
     */
    public static void checkWebdavCopyResource(Object webdavServlet, String source, String dest) {
        if (webdavServlet != null && source != null && dest != null) {
            String realPath = null;
            try {
                Object servletContext = Reflection.invokeMethod(webdavServlet, "getServletContext", new Class[]{});
                realPath = Reflection.invokeStringMethod(servletContext, "getRealPath", new Class[]{String.class}, "/");
                realPath = realPath.endsWith(System.getProperty("file.separator")) ? realPath.substring(0, realPath.length() - 1) : realPath;
            } catch (Exception e) {
                String message = e.getMessage();
                int errorCode = ErrorType.HOOK_ERROR.getCode();
                HookHandler.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
            }
            if (realPath != null) {
                HashMap<String, Object> params = new HashMap<String, Object>();
                params.put("source", realPath + source);
                params.put("dest", realPath + dest);
                HookHandler.doCheck(CheckParameter.Type.WEBDAV, params);
            }
        }
    }
}
