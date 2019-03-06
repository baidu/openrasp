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
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;
import org.mozilla.javascript.Scriptable;

import java.io.File;
import java.io.IOException;

/**
 * 　　* @Description: 文件改名hook点
 * 　　* @author anyang
 * 　　* @date 2018/7/23 10:54
 */
@HookAnnotation
public class FileRenameHook extends AbstractClassHook {
    @Override
    public boolean isClassMatched(String className) {
        return "java/io/File".equals(className);
    }

    @Override
    public String getType() {
        return "rename";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {

        String src = getInvokeStaticSrc(FileRenameHook.class, "checkFileRename", "$0,$1", File.class, File.class);
        insertBefore(ctClass, "renameTo", "(Ljava/io/File;)Z", src);
    }

    public static void checkFileRename(File source, File dest) {
        boolean checkSwitch = Config.getConfig().getPluginFilter();
        if (source != null && !source.isDirectory() && dest != null && !dest.isDirectory()) {
            if (checkSwitch && !source.exists()){
                return;
            }
            JSContext cx = JSContextFactory.enterAndInitContext();
            Scriptable params = cx.newObject(cx.getScope());
            try {
                params.put("source", params, source.getCanonicalPath());
            } catch (IOException e) {
                params.put("source", params, source.getAbsolutePath());
            }

            try {
                params.put("dest", params, dest.getCanonicalPath());
            } catch (IOException e) {
                params.put("dest", params, dest.getAbsolutePath());
            }
            HookHandler.doCheck(CheckParameter.Type.RENAME, params);
        }
    }
}
