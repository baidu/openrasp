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
import com.baidu.openrasp.tool.StackTrace;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;
import java.util.HashMap;

import java.io.File;
import java.io.IOException;
import java.util.List;

/**
 * Created by zhuming01 on 5/16/17. All rights reserved
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
        boolean checkSwitch = Config.getConfig().getPluginFilter();
        if (file != null) {
            if (checkSwitch && !file.exists()) {
                return;
            }
            HashMap<String, Object> params = null;
            try {
                params = new HashMap<String, Object>();
                params.put("path", file.getPath());
                List<String> stackInfo = StackTrace.getStackTraceArray();
                params.put("stack", stackInfo);
                try {
                    params.put("realpath", file.getCanonicalPath());
                } catch (Exception e) {
                    params.put("realpath", file.getAbsolutePath());
                }
            } catch (Throwable t) {
                String message = t.getMessage();
                int errorCode = ErrorType.HOOK_ERROR.getCode();
                HookHandler.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), t);
            }
            HookHandler.doCheck(CheckParameter.Type.DIRECTORY, params);
        }
    }

}
