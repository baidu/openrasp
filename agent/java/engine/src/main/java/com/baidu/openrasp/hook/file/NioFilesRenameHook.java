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
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;
import java.nio.file.Path;
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
        String src = getInvokeStaticSrc(NioFilesRenameHook.class, "checkFileRename", "$1,$2", Path.class,Path.class);
        insertBefore(ctClass, "copy", "(Ljava/nio/file/Path;Ljava/nio/file/Path;[Ljava/nio/file/CopyOption;)Ljava/nio/file/Path;", src);
        insertBefore(ctClass, "move", "(Ljava/nio/file/Path;Ljava/nio/file/Path;[Ljava/nio/file/CopyOption;)Ljava/nio/file/Path;", src);

        String srcLink = getInvokeStaticSrc(NioFilesRenameHook.class, "checkFileLink", "$1,$2", Path.class,Path.class);
        insertBefore(ctClass, "createLink", "(Ljava/nio/file/Path;Ljava/nio/file/Path;)Ljava/nio/file/Path;", srcLink);
        //软连接  待确认
        //insertBefore(ctClass, "createSymbolicLink", "(Ljava/nio/file/Path;Ljava/nio/file/Path;[Ljava/nio/file/attribute/FileAttribute;)Ljava/nio/file/Path;", srcLink);

    }

    /**
     * copy move hook；logic same as rename
     *
     * @param pathSource 源文件路径
     * @param pathDest 目标文件路径
     */
    public static void checkFileRename(Path pathSource,Path pathDest) {
        boolean checkSwitch = Config.getConfig().getPluginFilter();
        if (pathSource != null && !pathSource.toFile().isDirectory() && pathDest != null && !pathDest.toFile().isDirectory()) {
            if (checkSwitch && !pathSource.toFile().exists()){
                return;
            }
            HashMap<String, Object> params = new HashMap<String, Object>();
            try {
                params.put("source", pathSource.toFile().getCanonicalPath());
            } catch (IOException e) {
                params.put("source", pathSource.toFile().getAbsolutePath());
            }

            try {
                params.put("dest", pathDest.toFile().getCanonicalPath());
            } catch (IOException e) {
                params.put("dest", pathDest.toFile().getAbsolutePath());
            }
            HookHandler.doCheck(CheckParameter.Type.RENAME, params);
        }
    }

    /**
     * link hook；swap the source and destination ；logic same as rename
     *
     * @param pathSource 源文件路径
     * @param pathDest 目标文件路径
     */
    public static void checkFileLink(Path pathDest,Path pathSource) {
        boolean checkSwitch = Config.getConfig().getPluginFilter();
        if (pathSource != null && !pathSource.toFile().isDirectory() && pathDest != null && !pathDest.toFile().isDirectory()) {
            if (checkSwitch && !pathSource.toFile().exists()){
                return;
            }
            HashMap<String, Object> params = new HashMap<String, Object>();
            try {
                params.put("source", pathSource.toFile().getCanonicalPath());
            } catch (IOException e) {
                params.put("source", pathSource.toFile().getAbsolutePath());
            }

            try {
                params.put("dest", pathDest.toFile().getCanonicalPath());
            } catch (IOException e) {
                params.put("dest", pathDest.toFile().getAbsolutePath());
            }
            HookHandler.doCheck(CheckParameter.Type.RENAME, params);
        }
    }
}
