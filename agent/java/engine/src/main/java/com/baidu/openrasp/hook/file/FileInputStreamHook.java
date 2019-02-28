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
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import com.google.gson.Gson;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;
import org.mozilla.javascript.Scriptable;

import java.io.File;
import java.io.IOException;

/**
 * Created by zhuming01 on 5/31/17.
 * All rights reserved
 */
@HookAnnotation
public class FileInputStreamHook extends AbstractClassHook {
    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "readFile";
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "java/io/FileInputStream".equals(className);
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(FileInputStreamHook.class, "checkReadFile", "$1", File.class);
        insertBefore(ctClass.getConstructor("(Ljava/io/File;)V"), src);
    }

    /**
     * 文件读取hook点
     *
     * @param file 文件对象
     */
    public static void checkReadFile(File file) {
        boolean checkSwitch = Config.getConfig().getPluginFilter();
        if (file != null) {
            JSContext cx = JSContextFactory.enterAndInitContext();
            Scriptable params = cx.newObject(cx.getScope());
            params.put("path", params, file.getPath());

            String path;
            try {
                path = file.getCanonicalPath();
            } catch (Exception e) {
                path = file.getAbsolutePath();
            }
            if (path.endsWith(".class") || !file.exists() && checkSwitch) {
                return;
            }
            params.put("realpath", params, FileUtil.getRealPath(file));

            String hookType = CheckParameter.Type.READFILE.getName();
            //如果在lru缓存中不进检测
            if (!Config.commonLRUCache.isContainsKey(hookType + new Gson().toJson(params))) {
                HookHandler.doCheck(CheckParameter.Type.READFILE, params);
            }
        }
    }
}
