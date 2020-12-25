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

package com.baidu.openrasp.hook.file;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.request.RequestFileItem;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;
import org.apache.commons.lang3.StringUtils;

import java.io.IOException;
import java.util.HashMap;
import java.util.LinkedList;
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
        return "fileUploadParam";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {

        String src = getInvokeStaticSrc(FileUploadHook.class, "cacheFileUploadParam", "$_", Object.class);
        insertAfter(ctClass, "parseRequest", null, src);

    }

    public static void cacheFileUploadParam(Object object) {
        List<Object> list = (List<Object>) object;
        if (list != null && !list.isEmpty()) {
            HashMap<String, String[]> formItemCache = new HashMap<String, String[]>();
            LinkedList<RequestFileItem> fileItemCache = new LinkedList<RequestFileItem>();
            for (Object item : list) {
                boolean isFormField = (Boolean) Reflection.invokeMethod(item, "isFormField", new Class[]{});
                if (isFormField) {
                    String fieldName = Reflection.invokeStringMethod(item, "getFieldName", new Class[]{});
                    String fieldValue = Reflection.invokeStringMethod(item, "getString", new Class[]{});
                    formItemCache.put(fieldName, new String[]{fieldValue});
                } else {
                    String name = Reflection.invokeStringMethod(item, "getFieldName", new Class[]{});
                    String filename = Reflection.invokeStringMethod(item, "getName", new Class[]{});
                    fileItemCache.add(new RequestFileItem(name, filename));
                }
            }
            HookHandler.requestCache.get().setFormItemCache(formItemCache);
            HookHandler.requestCache.get().setFileParamCache(fileItemCache);
        }
    }

    private static String getCharSet(Object fileItem) {
        String charSet = Reflection.invokeStringMethod(fileItem, "getCharSet", new Class[]{});
        if (charSet == null) {
            if (HookHandler.requestCache.get() != null) {
                charSet = HookHandler.requestCache.get().getCharacterEncoding();
            }
        }
        if (!StringUtils.isEmpty(charSet)) {
            return charSet;
        } else {
            return "UTF-8";
        }
    }
}
