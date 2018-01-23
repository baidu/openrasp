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

package com.baidu.openrasp.hook.file;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.js.engine.JSContext;
import com.baidu.openrasp.plugin.js.engine.JSContextFactory;
import com.baidu.openrasp.tool.hook.CustomLockObject;
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
     * @see com.baidu.openrasp.hook.AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "writeFile";
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
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
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(int, String, String, String, String[], MethodVisitor)
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
                        mv.visitTypeInsn(NEW, "com/baidu/openrasp/tool/hook/CustomLockObject");
                        mv.visitInsn(DUP);
                        mv.visitMethodInsn(INVOKESPECIAL, "com/baidu/openrasp/tool/hook/CustomLockObject",
                                "<init>", "()V", false);
                        mv.visitFieldInsn(PUTFIELD, "java/io/FileOutputStream", "closeLock", "Ljava/lang/Object;");
                        mv.visitVarInsn(ALOAD, 0);
                        mv.visitFieldInsn(GETFIELD, "java/io/FileOutputStream", "closeLock", "Ljava/lang/Object;");
                        mv.visitVarInsn(ALOAD, 3);
                        mv.visitMethodInsn(INVOKESTATIC, "com/baidu/openrasp/hook/file/FileOutputStream2Hook", "checkFileOutputStreamInit",
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
                    mv.visitMethodInsn(INVOKESTATIC, "com/baidu/openrasp/hook/file/FileOutputStream2Hook", "checkFileOutputStreamWrite",
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
