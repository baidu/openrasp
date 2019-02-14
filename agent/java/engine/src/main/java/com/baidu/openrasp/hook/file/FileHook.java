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
import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.js.engine.JSContext;
import com.baidu.openrasp.plugin.js.engine.JSContextFactory;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import com.baidu.openrasp.tool.StackTrace;
import com.google.gson.Gson;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;
import org.mozilla.javascript.Scriptable;

import java.io.File;
import java.io.IOException;
import java.util.List;

/**
 * Created by zhuming01 on 5/16/17.
 * All rights reserved
 */
@HookAnnotation
public class FileHook extends AbstractClassHook {
    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "directory";
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "java/io/File".equals(className);
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(FileHook.class, "checkListFiles", "$0", File.class);
        insertBefore(ctClass, "list", "()[Ljava/lang/String;", src);
    }

    /**
     * 列出文件列表方法 hook 点检测入口
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
                } catch (Exception e) {
                    params.put("realpath", params, file.getAbsolutePath());
                }
            } catch (Throwable t) {
                String message = t.getMessage();
                int errorCode = ErrorType.HOOK_ERROR.getCode();
                HookHandler.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), t);
            }
            String hookType = CheckParameter.Type.DIRECTORY.getName();
            //如果在lru缓存中不进检测
            if (params != null && !HookHandler.commonLRUCache.isContainsKey(hookType + new Gson().toJson(params))) {
                HookHandler.doCheck(CheckParameter.Type.DIRECTORY, params);
            }
        }
    }

}
