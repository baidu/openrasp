/*
 * Copyright 2017-2020 Baidu Inc.
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
import com.baidu.openrasp.messaging.LogTool;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.StackTrace;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.List;

/**
 * nio files list hook
 * liergou
 * 2020.7.10
 */
@HookAnnotation
public class NioFilesListHook extends AbstractClassHook {
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
        return "java/nio/file/Files".equals(className);
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(NioFilesListHook.class, "checkFileList", "$1", Object.class);
        insertBefore(ctClass, "newDirectoryStream", "(Ljava/nio/file/Path;)Ljava/nio/file/DirectoryStream;", src);
//        insertBefore(ctClass, "walk", "(Ljava/nio/file/Path;I[Ljava/nio/file/FileVisitOption;)Ljava/util/stream/Stream;", src);
//        insertBefore(ctClass, "walkFileTree", "(Ljava/nio/file/Path;Ljava/util/Set;ILjava/nio/file/FileVisitor;)Ljava/nio/file/Path;", src);
//        insertBefore(ctClass, "find", "(Ljava/nio/file/Path;ILjava/util/function/BiPredicate;[Ljava/nio/file/FileVisitOption;)Ljava/util/stream/Stream;", src);
    }

    /**
     * nio file list hook
     *
     * @param path nio file path
     */
    public static void checkFileList(Object path) {
        boolean checkSwitch = Config.getConfig().getPluginFilter();
        if (path != null) {
            File file = (File) Reflection.invokeMethod(path, "toFile", new Class[]{});
            if (checkSwitch && !file.exists()) {
                return;
            }
            HashMap<String, Object> params = null;
            try {
                params = new HashMap<String, Object>();
                params.put("path", file.getPath());
                List<String> stackInfo = StackTrace.getParamStackTraceArray();
                params.put("stack", stackInfo);
                try {
                    params.put("realpath", file.getCanonicalPath());
                } catch (Exception e) {
                    params.put("realpath", file.getAbsolutePath());
                }
            } catch (Throwable t) {
                LogTool.traceHookWarn(t.getMessage(), t);
            }
            HookHandler.doCheck(CheckParameter.Type.DIRECTORY, params);
        }
    }

}
