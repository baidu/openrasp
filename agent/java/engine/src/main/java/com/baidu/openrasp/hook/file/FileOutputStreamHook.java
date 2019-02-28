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
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.js.engine.JSContext;
import com.baidu.openrasp.plugin.js.engine.JSContextFactory;
import com.baidu.openrasp.tool.FileUtil;
import com.baidu.openrasp.tool.StackTrace;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import com.google.gson.Gson;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;
import org.mozilla.javascript.Scriptable;

import java.io.File;
import java.io.IOException;
import java.util.List;

/**
 * Created by lxk on 6/8/17.
 * <p>
 * 文件输出流 hook 点
 */
@HookAnnotation
public class FileOutputStreamHook extends AbstractClassHook {
    /**
     * (none-javadoc)
     *
     * @see AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "writeFile";
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "java/io/FileOutputStream".equals(className);
    }


    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(FileOutputStreamHook.class, "checkWriteFile", "$1", File.class);
        insertBefore(ctClass.getConstructor("(Ljava/io/File;Z)V"), src);
    }

    /**
     * 写文件hook点
     *
     * @param file
     */
    public static void checkWriteFile(File file) {
        if (file != null) {
            JSContext cx = JSContextFactory.enterAndInitContext();
            Scriptable params = cx.newObject(cx.getScope());
            params.put("path", params, file.getName());
            params.put("realpath", params, FileUtil.getRealPath(file));
            List<String> stackInfo = StackTrace.getStackTraceArray(Config.REFLECTION_STACK_START_INDEX,
                    Config.getConfig().getPluginMaxStack());
            Scriptable stackArray = cx.newArray(cx.getScope(), stackInfo.toArray());
            params.put("stack", params, stackArray);
            String hookType = CheckParameter.Type.WRITEFILE.getName();
            //如果在lru缓存中不进检测
            if (!Config.commonLRUCache.isContainsKey(hookType + new Gson().toJson(params))) {
                HookHandler.doCheck(CheckParameter.Type.WRITEFILE, params);
            }
        }
    }

}
