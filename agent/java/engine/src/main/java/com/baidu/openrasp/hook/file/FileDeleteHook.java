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
 * @description: 任意文件删除hook点
 * @author: anyang
 * @create: 2019/04/19 18:01
 */
@HookAnnotation
public class FileDeleteHook extends AbstractClassHook {
    @Override
    public boolean isClassMatched(String className) {
        return "java/io/File".equals(className);
    }

    @Override
    public String getType() {
        return "deleteFile";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(FileDeleteHook.class, "checkDeleteFile", "$0", File.class);
        insertBefore(ctClass, "delete", "()Z", src);
    }

    public static void checkDeleteFile(File file) {
        if (file != null) {
            JSContext cx = JSContextFactory.enterAndInitContext();
            Scriptable params = cx.newObject(cx.getScope());
            String path = file.getPath();
            params.put("path", params, path);
            try {
                params.put("realpath", params, file.getCanonicalPath());
            } catch (IOException e) {
                params.put("realpath", params, file.getAbsolutePath());
            }
            HookHandler.doCheck(CheckParameter.Type.DELETEFILE, params);
        }
    }
}
