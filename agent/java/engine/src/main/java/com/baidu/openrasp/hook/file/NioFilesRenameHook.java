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
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.File;
import java.io.IOException;
import java.util.HashMap;

/**
 * nio file move and copy and link hook
 * liergou
 * 2020.07.10
 */
@HookAnnotation
public class NioFilesRenameHook extends AbstractClassHook {
    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "rename";
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
        String src = getInvokeStaticSrc(NioFilesRenameHook.class, "checkFileRename", "$1,$2", Object.class, Object.class);
        insertBefore(ctClass, "copy", "(Ljava/nio/file/Path;Ljava/nio/file/Path;[Ljava/nio/file/CopyOption;)Ljava/nio/file/Path;", src);
        insertBefore(ctClass, "move", "(Ljava/nio/file/Path;Ljava/nio/file/Path;[Ljava/nio/file/CopyOption;)Ljava/nio/file/Path;", src);

        String srcLinkHard = getInvokeStaticSrc(NioFilesRenameHook.class, "checkFileLink", "$1,$2," + "\"hard\"", Object.class, Object.class,String.class);
        insertBefore(ctClass, "createLink", "(Ljava/nio/file/Path;Ljava/nio/file/Path;)Ljava/nio/file/Path;", srcLinkHard);
//        String srcLinkSoft = getInvokeStaticSrc(NioFilesRenameHook.class, "checkFileLink", "$1,$2," + "\"soft\"", Object.class, Object.class);
//        insertBefore(ctClass, "createSymbolicLink", "(Ljava/nio/file/Path;Ljava/nio/file/Path;[Ljava/nio/file/attribute/FileAttribute;)Ljava/nio/file/Path;", srcLinkSoft);

    }

    /**
     * copy move hook；logic same as rename
     *
     * @param pathSource 源文件路径
     * @param pathDest   目标文件路径
     */
    public static void checkFileRename(Object pathSource, Object pathDest) {
        boolean checkSwitch = Config.getConfig().getPluginFilter();
        File fileSource = null;
        File fileDest = null;
        if (pathSource != null && pathDest != null) {
            fileSource = (File) Reflection.invokeMethod(pathSource, "toFile", new Class[]{});
            fileDest = (File) Reflection.invokeMethod(pathDest, "toFile", new Class[]{});
        }
        if (fileSource != null && fileDest != null && !fileSource.isDirectory() && !fileDest.isDirectory()) {
            if (checkSwitch && !fileSource.exists()) {
                return;
            }
            HashMap<String, Object> params = new HashMap<String, Object>();
            try {
                params.put("source", fileSource.getCanonicalPath());
            } catch (IOException e) {
                params.put("source", fileSource.getAbsolutePath());
            }

            try {
                params.put("dest", fileDest.getCanonicalPath());
            } catch (IOException e) {
                params.put("dest", fileDest.getAbsolutePath());
            }
            HookHandler.doCheck(CheckParameter.Type.RENAME, params);
        }
    }

    /**
     * link hook；swap the source and destination ；logic same as rename
     *
     * @param pathSource 源文件路径
     * @param pathDest   目标文件路径
     * @param type       链接类型
     */
    public static void checkFileLink(Object pathDest, Object pathSource, String type) {
        File fileSource = null;
        File fileDest = null;
        if (pathSource != null && pathDest != null) {
            fileSource = (File) Reflection.invokeMethod(pathSource, "toFile", new Class[]{});
            fileDest = (File) Reflection.invokeMethod(pathDest, "toFile", new Class[]{});
        }
        if (fileSource != null && fileDest != null && !fileSource.isDirectory() && !fileDest.isDirectory()) {
            if (Config.getConfig().getPluginFilter() && !fileSource.exists()) {
                return;
            }
            HashMap<String, Object> params = new HashMap<String, Object>();
            params.put("type", type);
            try {
                params.put("source", fileSource.getCanonicalPath());
            } catch (IOException e) {
                params.put("source", fileSource.getAbsolutePath());
            }

            try {
                params.put("dest", fileDest.getCanonicalPath());
            } catch (IOException e) {
                params.put("dest", fileDest.getAbsolutePath());
            }
            HookHandler.doCheck(CheckParameter.Type.LINK, params);
        }
    }
}
