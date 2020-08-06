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
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.FileUtil;
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
 * nio files write hook
 * liergou
 * 2020.7.9
 */
@HookAnnotation
public class NioFilesWriteHook extends AbstractClassHook {
    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "writeFile";
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
        String src = getInvokeStaticSrc(NioFilesWriteHook.class, "checkNioWriteFile", "$1", Object.class);
        insertBefore(ctClass, "createFile", "(Ljava/nio/file/Path;[Ljava/nio/file/attribute/FileAttribute;)Ljava/nio/file/Path;", src);
        insertBefore(ctClass, "newOutputStream", "(Ljava/nio/file/Path;[Ljava/nio/file/OpenOption;)Ljava/io/OutputStream;", src);
        //创建目录  待确认
        //insertBefore(ctClass, "createDirectory", "(Ljava/nio/file/Path;[Ljava/nio/file/attribute/FileAttribute;)Ljava/nio/file/Path;", src);
        //读写channel,一般不会直接使用，暂不hook
        //insertBefore(ctClass, "newByteChannel", "(Ljava/nio/file/Path;Ljava/util/Set;[Ljava/nio/file/attribute/FileAttribute;)Ljava/nio/channels/SeekableByteChannel;", src);
        //insertBefore(ctClass, "newByteChannel", "(Ljava/nio/file/Path;[Ljava/nio/file/OpenOption;)Ljava/nio/channels/SeekableByteChannel;", src);
    }

    /**
     * nio files write hook
     *
     * @param path 文件路径
     */
    public static void checkNioWriteFile(Object path) {
        if (path != null) {
            File file = (File) Reflection.invokeMethod(path, "toFile", new Class[]{});
            HashMap<String, Object> params = new HashMap<String, Object>();
            params.put("path", file.getPath());
            params.put("realpath", FileUtil.getRealPath(file));
            List<String> stackInfo = StackTrace.getParamStackTraceArray();
            params.put("stack", stackInfo);
            HookHandler.doCheck(CheckParameter.Type.WRITEFILE, params);
        }
    }
}
