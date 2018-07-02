/*
 * Copyright 2017-2018 Baidu Inc.
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

import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.hook.file.FileInputStreamHook;
import com.baidu.openrasp.tool.Reflection;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;
import org.apache.commons.io.FilenameUtils;

import java.io.File;
import java.io.IOException;

/**
 * Created by zhuming01 on 5/31/17.
 * All rights reserved
 */
public class ProxyDirContextHook extends AbstractClassHook {
    /**
     * (none-javadoc)
     *
     * @see AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "readFile";
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/naming/resources/ProxyDirContext".equals(className);
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(ProxyDirContextHook.class, "checkResourceCacheEntry",
                "$_", Object.class);
        insertAfter(ctClass, "lookup", "(Ljava/lang/String;)Ljava/lang/Object;", src);
    }

    /**
     * 缓存资源文件读取hook点
     *
     * @param resource 文件资源
     */
    public static void checkResourceCacheEntry(Object resource) {
        if (resource != null) {
            Object file = null;
            try {
                if (resource.getClass().getName().startsWith("org.apache.naming.resources."
                        + "FileDirContext$FileResource")) {
                    Object binaryContent = Reflection.invokeMethod(resource, "getContent", new Class[]{});
                    if (null == binaryContent) {
                        file = Reflection.getField(resource, "file");
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
            if (null != file && file instanceof File) {
                String filename = ((File) file).getName();
                if (FilenameUtils.getExtension(filename).matches(Config.getConfig().getReadFileExtensionRegex())) {
                    FileInputStreamHook.checkReadFile((File) file);
                }
            }
        }
    }
}
