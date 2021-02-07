/*
 * Copyright 2017-2021 Baidu Inc.
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

package com.baidu.openrasp.hook.server.resin;

import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.hook.server.ServerInputHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * Created by tyy on 18-2-11.
 *
 * 获取 resin 请求 body 的 hook 点
 */
@HookAnnotation
public class ResinHttpInputHook extends ServerInputHook {

    /**
     * (none-javadoc)
     *
     * @see AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return className.equals("com/caucho/server/connection/ServletInputStreamImpl")
                || className.equals("com/caucho/server/http/ServletInputStreamImpl")
                || className.equals("com/caucho/vfs/BufferedReaderAdapter");
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        if (ctClass.getName().contains("BufferedReaderAdapter")) {
            String src = getInvokeStaticSrc(ServerInputHook.class, "onCharRead",
                    "$_,$0", int.class, Object.class);
            insertAfter(ctClass, "read", "()I", src);
            src = getInvokeStaticSrc(ServerInputHook.class, "onCharRead",
                    "$_,$0,$1,$2", int.class, Object.class, char[].class, int.class);
            insertAfter(ctClass, "read", "([CII)I", src);
            src = getInvokeStaticSrc(ServerInputHook.class, "onCharReadLine",
                    "$_,$0", String.class, Object.class);
            insertAfter(ctClass, "readLine", "()Ljava/lang/String;", src);
        } else {
            String src = getInvokeStaticSrc(ServerInputHook.class, "onInputStreamRead",
                    "$_,$0", int.class, Object.class);
            insertAfter(ctClass, "read", "()I", src);
            src = getInvokeStaticSrc(ServerInputHook.class, "onInputStreamRead",
                    "$_,$0,$1,$2", int.class, Object.class, byte[].class, int.class);
            insertAfter(ctClass, "read", "([BII)I", src);
        }
    }

}
