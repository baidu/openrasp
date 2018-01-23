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
import org.mozilla.javascript.Scriptable;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;
import org.objectweb.asm.commons.AdviceAdapter;
import org.objectweb.asm.commons.Method;

import java.io.UnsupportedEncodingException;
import java.util.Arrays;

/**
 * Created by zhuming01 on 5/5/17.
 * All rights reserved
 */
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

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(int, String, String, String, String[], MethodVisitor) (String)
     */
    @Override
    public MethodVisitor hookMethod(int access, String name, String desc, String signature, String[] exceptions, MethodVisitor mv) {
        if (name.equals("setHeaders")) {
            return new AdviceAdapter(Opcodes.ASM5, mv, access, name, desc) {
                @Override
                protected void onMethodExit(int opcode) {
                    loadThis();
                    invokeInterface(Type.getType("org/apache/commons/fileupload/FileItem"),
                            new Method("getName", "()Ljava/lang/String;"));

                    loadThis();
                    invokeInterface(Type.getType("org/apache/commons/fileupload/FileItem"),
                            new Method("get", "()[B"));

                    invokeStatic(Type.getType(DiskFileItemHook.class),
                            new Method("checkFileUpload", "(Ljava/lang/String;[B)V"));

                    super.onMethodExit(opcode);
                }
            };
        }
        return mv;
    }

    /**
     * 文件上传hook点
     *
     * @param name    文件名
     * @param content 文件数据
     */
    public static void checkFileUpload(String name, byte[] content) {
        if (name != null && content != null) {
            JSContext cx = JSContextFactory.enterAndInitContext();
            Scriptable params = cx.newObject(cx.getScope());
            params.put("filename", params, name);
            try {
                if (content.length > 4 * 1024) {
                    content = Arrays.copyOf(content, 4 * 1024);
                }
                params.put("content", params, new String(content, "UTF-8"));
            } catch (UnsupportedEncodingException e) {
                e.printStackTrace();
                params.put("content", params, "[rasp error:" + e.getMessage() + "]");
            }

            HookHandler.doCheck(CheckParameter.Type.FILEUPLOAD, params);
        }
    }
}
