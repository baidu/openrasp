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

package com.fuxi.javaagent.hook.file;

import com.fuxi.javaagent.HookHandler;
import com.fuxi.javaagent.config.Config;
import com.fuxi.javaagent.hook.AbstractClassHook;
import com.fuxi.javaagent.plugin.checker.CheckParameter;
import com.fuxi.javaagent.plugin.js.engine.JSContext;
import com.fuxi.javaagent.plugin.js.engine.JSContextFactory;
import com.fuxi.javaagent.tool.StackTrace;
import org.mozilla.javascript.Scriptable;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;
import org.objectweb.asm.commons.AdviceAdapter;
import org.objectweb.asm.commons.Method;

import java.io.File;
import java.io.IOException;
import java.util.List;

/**
 * Created by zhuming01 on 5/16/17.
 * All rights reserved
 */
public class FileHook extends AbstractClassHook {
    /**
     * (none-javadoc)
     *
     * @see com.fuxi.javaagent.hook.AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "directory";
    }

    /**
     * (none-javadoc)
     *
     * @see com.fuxi.javaagent.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "java/io/File".equals(className);
    }

    /**
     * (none-javadoc)
     *
     * @see com.fuxi.javaagent.hook.AbstractClassHook#hookMethod(int, String, String, String, String[], MethodVisitor)
     */
    @Override
    public MethodVisitor hookMethod(int access, String name, String desc, String signature, String[] exceptions, MethodVisitor mv) {
        if (name.equals("listFiles")) {
            return new AdviceAdapter(Opcodes.ASM5, mv, access, name, desc) {
                @Override
                protected void onMethodEnter() {
                    loadThis();
                    invokeStatic(Type.getType(FileHook.class),
                            new Method("checkListFiles", "(Ljava/io/File;)V"));
                }
            };
        }
        return mv;
    }

    /**
     * 列出文件列表方法hook点
     *
     * @param file 文件对象
     */
    public static void checkListFiles(File file) {
        if (file != null) {
            Scriptable params = null;
            try {
                JSContext cx = JSContextFactory.enterAndInitContext();
                params = cx.newObject(cx.getScope());
                params.put("path", params, file.getPath());
                List<String> stackInfo = StackTrace.getStackTraceArray(Config.REFLECTION_STACK_START_INDEX,
                        Config.getConfig().getPluginMaxStack());
                Scriptable stackArray = cx.newArray(cx.getScope(), stackInfo.toArray());
                params.put("stack", params, stackArray);
                try {
                    params.put("realpath", params, file.getCanonicalPath());
                } catch (IOException e) {
                    params.put("realpath", params, file.getAbsolutePath());
                }
            } catch (Throwable t) {
                HookHandler.LOGGER.warn(t.getMessage());
            }
            if (params != null) {
                HookHandler.doCheck(CheckParameter.Type.DIRECTORY, params);
            }
        }
    }

}
