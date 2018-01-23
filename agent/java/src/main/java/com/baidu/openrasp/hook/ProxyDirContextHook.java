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
import org.apache.commons.io.FilenameUtils;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.commons.AdviceAdapter;

import java.io.File;

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
     * @see AbstractClassHook#hookMethod(int, String, String, String, String[], MethodVisitor)
     */
    @Override
    protected MethodVisitor hookMethod(int access, String name, String desc, String signature, String[] exceptions, MethodVisitor mv) {
        if ("lookup".equals(name) && desc.startsWith("(Ljava/lang/String;)")) {
            return new AdviceAdapter(Opcodes.ASM5, mv, access, name, desc) {
                @Override
                public void onMethodExit(int opcode) {
                    if (opcode == Opcodes.ARETURN) {
                        mv.visitVarInsn(ALOAD, 2);
                        mv.visitMethodInsn(INVOKESTATIC, "com/baidu/openrasp/hook/ProxyDirContextHook", "checkResourceCacheEntry",
                                "(Ljava/lang/Object;)V", false);
                    }
                    super.onMethodExit(opcode);
                }

            };
        }
        return mv;
    }

    /**
     * 缓存资源文件读取hook点
     *
     * @param cacheEntry
     */
    public static void checkResourceCacheEntry(Object cacheEntry) {
        if (cacheEntry != null) {
            Object file = null;
            try {
                Object resource = Reflection.getField(cacheEntry, "resource");
                if (null != resource && resource.getClass().getName().startsWith("org.apache.naming.resources."
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
