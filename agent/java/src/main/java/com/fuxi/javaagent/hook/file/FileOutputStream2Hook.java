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

package com.fuxi.javaagent.hook.file;

import com.fuxi.javaagent.HookHandler;
import com.fuxi.javaagent.hook.AbstractClassHook;
import com.fuxi.javaagent.plugin.checker.CheckParameter;
import com.fuxi.javaagent.plugin.js.engine.JSContext;
import com.fuxi.javaagent.plugin.js.engine.JSContextFactory;
import com.fuxi.javaagent.tool.hook.CustomLockObject;
import org.mozilla.javascript.Scriptable;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.commons.AdviceAdapter;

import java.io.File;

/**
 * Created by lxk on 6/6/17.
 */
public class FileOutputStream2Hook extends AbstractClassHook {
    /**
     * (none-javadoc)
     *
     * @see com.fuxi.javaagent.hook.AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "writeFile";
    }

    /**
     * (none-javadoc)
     *
     * @see com.fuxi.javaagent.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "java/io/FileOutputStream".equals(className);
    }


    /**
     * (none-javadoc)
     *
     * @see AbstractClassHook#computeFrames()
     */
    @Override
    protected boolean computeFrames() {
        return true;
    }

    /**
     * (none-javadoc)
     *
     * @see com.fuxi.javaagent.hook.AbstractClassHook#hookMethod(int, String, String, String, String[], MethodVisitor)
     */
    @Override
    protected MethodVisitor hookMethod(int access, String name, String desc, String signature, String[] exceptions, MethodVisitor mv) {
        // store file info in lockClose Object
        if ("<init>".equals(name) && "(Ljava/io/File;Z)V".equals(desc)) {
            return new AdviceAdapter(Opcodes.ASM5, mv, access, name, desc) {
                @Override
                public void onMethodExit(int opcode) {
                    if (opcode == Opcodes.RETURN) {
                        mv.visitVarInsn(ALOAD, 0);
                        mv.visitTypeInsn(NEW, "com/fuxi/javaagent/tool/hook/CustomLockObject");
                        mv.visitInsn(DUP);
                        mv.visitMethodInsn(INVOKESPECIAL, "com/fuxi/javaagent/tool/hook/CustomLockObject",
                                "<init>", "()V", false);
                        mv.visitFieldInsn(PUTFIELD, "java/io/FileOutputStream", "closeLock", "Ljava/lang/Object;");
                        mv.visitVarInsn(ALOAD, 0);
                        mv.visitFieldInsn(GETFIELD, "java/io/FileOutputStream", "closeLock", "Ljava/lang/Object;");
                        mv.visitVarInsn(ALOAD, 3);
                        mv.visitMethodInsn(INVOKESTATIC, "com/fuxi/javaagent/hook/file/FileOutputStream2Hook", "checkFileOutputStreamInit",
                                "(Ljava/lang/Object;Ljava/lang/String;)V", false);
                    }
                    super.onMethodExit(opcode);
                }
            };
        }
        if (name.equals("write") && desc.startsWith("([B")) {
            return new AdviceAdapter(Opcodes.ASM5, mv, access, name, desc) {
                @Override
                public void onMethodEnter() {
                    mv.visitVarInsn(ALOAD, 0);
                    mv.visitFieldInsn(GETFIELD, "java/io/FileOutputStream", "closeLock", "Ljava/lang/Object;");
                    mv.visitVarInsn(ALOAD, 1);
                    mv.visitMethodInsn(INVOKESTATIC, "com/fuxi/javaagent/hook/file/FileOutputStream2Hook", "checkFileOutputStreamWrite",
                            "(Ljava/lang/Object;[B)V", false);
                }
            };
        }
        return mv;
    }

    /**
     * 文件输出流的构造函数hook点
     *
     * @param closeLock 用于记录文件信息的锁对象
     * @param path      文件路径
     */
    public static void checkFileOutputStreamInit(Object closeLock, String path) {
        if (closeLock instanceof CustomLockObject && HookHandler.enableHook.get() && HookHandler.isEnableCurrThreadHook()) {
            ((CustomLockObject) closeLock).setInfo(path);
        }
    }

    /**
     * 文件写入hook点
     *
     * @param closeLock  缓存文件信息
     * @param writeBytes 写的内容
     */
    public static void checkFileOutputStreamWrite(Object closeLock, byte[] writeBytes) {
        if (closeLock instanceof CustomLockObject && ((CustomLockObject) closeLock).getInfo() != null) {
            String path = ((CustomLockObject) closeLock).getInfo();
            if (path != null && writeBytes != null && writeBytes.length > 0) {
                File file = new File(path);
                JSContext cx = JSContextFactory.enterAndInitContext();
                Scriptable params = cx.newObject(cx.getScope());
                params.put("name", params, file.getName());
                params.put("realpath", params, path);
                params.put("content", params, new String(writeBytes));
                HookHandler.doCheck(CheckParameter.Type.WRITEFILE, params);
            }
        }
    }
}
