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
import org.apache.commons.lang3.StringUtils;

import java.io.IOException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;

/**
 * @author anyang
 * @Description: 获取文件上传参数hook点
 * @date 2018/7/5 15:13
 */
@HookAnnotation
public class FileUploadHook extends AbstractClassHook {

    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/commons/fileupload/FileUploadBase".equals(className);
    }

    @Override
    public String getType() {
        return "fileUpload";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {

        String src = getInvokeStaticSrc(FileUploadHook.class, "cacheFileUploadParam", "$_", Object.class);
        insertAfter(ctClass, "parseRequest", null, src);

    }

    public static void cacheFileUploadParam(Object object) {
        List<Object> list = (List<Object>) object;
        if (list != null && !list.isEmpty()) {
            HashMap<String, Object> params = new HashMap<String, Object>();
            HashMap<String, String[]> fileUploadCache = new HashMap<String, String[]>();
            for (Object o : list) {
                boolean isFormField = (Boolean) Reflection.invokeMethod(o, "isFormField", new Class[]{});
                if (isFormField) {
                    String fieldName = Reflection.invokeStringMethod(o, "getFieldName", new Class[]{});
                    String fieldValue = Reflection.invokeStringMethod(o, "getString", new Class[]{});
                    fileUploadCache.put(fieldName, new String[]{fieldValue});
                } else {
                    String name = Reflection.invokeStringMethod(o, "getFieldName", new Class[]{});
                    params.put("name", name != null ? name : "");
                    String filename = Reflection.invokeStringMethod(o, "getName", new Class[]{});
                    params.put("filename", filename);
                    byte[] content = (byte[]) Reflection.invokeMethod(o, "get", new Class[]{});
                    if (content.length > 4 * 1024) {
                        content = Arrays.copyOf(content, 4 * 1024);
                    }
                    try {
                        params.put("content", new String(content, getCharSet(o)));
                    } catch (Exception e) {
                        params.put("content", new String(content));
                        String message = e.getMessage();
                        int errorCode = ErrorType.HOOK_ERROR.getCode();
                        HookHandler.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
                    }
                }
            }
            //只缓存multipart中的非文件字段值
            HookHandler.requestCache.get().setFileUploadCache(fileUploadCache);
            if (!params.isEmpty()) {
                HookHandler.doCheck(CheckParameter.Type.FILEUPLOAD, params);
            }
        }
    }

    private static String getCharSet(Object fileItem) {
        String charSet = Reflection.invokeStringMethod(fileItem, "getCharSet", new Class[]{});
        if (charSet == null) {
            charSet = HookHandler.requestCache.get().getCharacterEncoding();
        }
        if (!StringUtils.isEmpty(charSet)) {
            return charSet;
        } else {
            return "UTF-8";
        }
    }
}
