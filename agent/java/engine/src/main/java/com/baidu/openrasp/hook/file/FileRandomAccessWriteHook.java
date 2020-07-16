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

import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.File;
import java.io.IOException;

/**
 * @description: RandomAccessFile 写文件 hook点
 * @author: anyang
 * @create: 2019/05/28 17:23
 */
@HookAnnotation
public class FileRandomAccessWriteHook extends AbstractClassHook {
    @Override
    public boolean isClassMatched(String className) {
        return "java/io/RandomAccessFile".equals(className);
    }

    @Override
    public String getType() {
        return "writeFile";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String writeSrc = getInvokeStaticSrc(FileRandomAccessWriteHook.class, "checkWriteFile", "$1,$2", File.class, String.class);
        insertBefore(ctClass.getConstructor("(Ljava/io/File;Ljava/lang/String;)V"), writeSrc);
    }

    public static void checkWriteFile(File file, String mode) {
        if (mode.startsWith("rw")) {
            if (file != null && !file.getName().endsWith(".jar") && !file.getName().endsWith(".war")) {
                FileOutputStreamHook.checkWriteFile(file);
            }
        }
    }
}
