/*
 * Copyright 2017-2018 Baidu Inc.
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

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.hook.server.ServerInputHook;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * Created by tyy on 18-2-11.
 *
 * 获取 resin 请求 body 的 hook 点
 */
public class ResinHttpInputHook extends ServerInputHook {

    /**
     * (none-javadoc)
     *
     * @see AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return className.equals("com/caucho/server/connection/ServletInputStreamImpl")
                || className.equals("com/caucho/server/http/ServletInputStreamImpl");
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String srcRead1 = getInvokeStaticSrc(HookHandler.class, "onInputStreamRead",
                "$_,$0", int.class, Object.class);
        insertAfter(ctClass, "read", "()I", srcRead1);
        String src2Read2 = getInvokeStaticSrc(HookHandler.class, "onInputStreamRead",
                "$_,$0,$1,$2,$3", int.class, Object.class, byte[].class, int.class, int.class);
        insertAfter(ctClass, "read", "([BII)I", src2Read2);
    }

}
