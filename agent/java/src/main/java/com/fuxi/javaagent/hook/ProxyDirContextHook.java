/**
 * Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

package com.fuxi.javaagent.hook;

import com.fuxi.javaagent.config.Config;
import com.fuxi.javaagent.hook.file.FileInputStreamHook;
import com.fuxi.javaagent.tool.Reflection;
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
                        mv.visitMethodInsn(INVOKESTATIC, "com/fuxi/javaagent/hook/ProxyDirContextHook", "checkResourceCacheEntry",
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
