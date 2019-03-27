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

package com.baidu.openrasp.hook.file;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.js.engine.JSContext;
import com.baidu.openrasp.plugin.js.engine.JSContextFactory;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;
import org.mozilla.javascript.Scriptable;

import java.io.IOException;
import java.util.Arrays;

/**
 * Created by zhuming01 on 5/5/17.
 * All rights reserved
 */
@HookAnnotation
public class DiskFileItemHook extends AbstractClassHook {
    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "fileUpload";
    }


    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/commons/fileupload/disk/DiskFileItem".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(DiskFileItemHook.class, "checkFileUpload",
                "getName(),get(),$0", String.class, byte[].class, Object.class);
        insertAfter(ctClass, "setHeaders", null, src, true);
    }

    /**
     * 文件上传hook点
     *
     * @param name    文件名
     * @param content 文件数据
     */
    public static void checkFileUpload(String name, byte[] content, Object file) {
        if (name != null && content != null && file != null) {
            JSContext cx = JSContextFactory.enterAndInitContext();
            Scriptable params = cx.newObject(cx.getScope());
            params.put("filename", params, name);
            try {
                if (content.length > 4 * 1024) {
                    content = Arrays.copyOf(content, 4 * 1024);
                }
                String characterEncoding = "UTF-8";
                if (HookHandler.requestCache.get() != null) {
                    String encoding = HookHandler.requestCache.get().getCharacterEncoding();
                    if (encoding != null && !encoding.isEmpty()) {
                        characterEncoding = encoding;
                    }
                }
                params.put("content", params, new String(content, characterEncoding));
            } catch (Exception e) {
                String message = e.getMessage();
                int errorCode = ErrorType.HOOK_ERROR.getCode();
                HookHandler.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
                params.put("content", params, "");
            }
            boolean isFiled = (Boolean) Reflection.invokeMethod(file, "isFormField", new Class[]{});
            if (!isFiled) {
                String fileName = Reflection.invokeStringMethod(file, "getFieldName", new Class[]{});
                params.put("name", params, fileName != null ? fileName : "");
            }

            HookHandler.doCheck(CheckParameter.Type.FILEUPLOAD, params);
        }
    }
}
