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
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;
import org.apache.commons.io.IOUtils;

import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.HashMap;

/**
 * @description: Jersey 文件上传
 * @author: anyang
 * @create: 2019/01/28 12:43
 */
@HookAnnotation
public class JerseyMultipart extends AbstractClassHook {
    @Override
    public boolean isClassMatched(String className) {
        return "org/glassfish/jersey/media/multipart/FormDataMultiPart".equals(className);
    }

    @Override
    public String getType() {
        return "fileUpload";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(JerseyMultipart.class, "checkFileUpload", "$_", Object.class);
        insertAfter(ctClass, "getFields", "()Ljava/util/Map;", src);
    }

    public static void checkFileUpload(Object result) {
        if (result != null) {
            Map<String, List<Object>> map = (Map<String, List<Object>>) result;
            for (Map.Entry<String, List<Object>> entry : map.entrySet()) {
                Object o = entry.getValue().get(0);
                Object contentDisposition = Reflection.invokeMethod(o, "getFormDataContentDisposition", new Class[]{});
                String name = Reflection.invokeStringMethod(contentDisposition, "getFileName", new Class[]{});
                if (name != null) {
                    HashMap<String, Object> params = new HashMap<String, Object>();
                    params.put("filename", name);
                    InputStream inputStream = (InputStream) Reflection.invokeMethod(o, "getValueAs", new Class[]{Class.class}, InputStream.class);
                    try {
                        byte[] content = IOUtils.toByteArray(inputStream);
                        if (content.length > 4 * 1024) {
                            content = Arrays.copyOf(content, 4 * 1024);
                        }
                        params.put("content", new String(content));
                    } catch (IOException e) {
                        String message = e.getMessage();
                        int errorCode = ErrorType.HOOK_ERROR.getCode();
                        HookHandler.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
                        params.put("content", "[rasp error:" + e.getMessage() + "]");
                    }
                    params.put("name", "");
                    HookHandler.doCheck(CheckParameter.Type.FILEUPLOAD, params);
                }
            }
        }
    }
}
