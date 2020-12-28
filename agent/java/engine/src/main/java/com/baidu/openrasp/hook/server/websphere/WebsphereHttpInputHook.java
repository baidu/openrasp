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

package com.baidu.openrasp.hook.server.websphere;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.server.ServerInputHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * @author anyang
 * @Description: 获取websphere的body
 * @date 2018/8/13 17:18
 */
@HookAnnotation
public class WebsphereHttpInputHook extends ServerInputHook {
    @Override
    public boolean isClassMatched(String className) {
        return "com/ibm/ws/webcontainer/srt/http/HttpInputStream".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String readByteSrc = getInvokeStaticSrc(ServerInputHook.class, "onInputStreamRead",
                "$_,$0", int.class, Object.class);
        insertAfter(ctClass, "read", "()I", readByteSrc);
        String readSrc = getInvokeStaticSrc(ServerInputHook.class, "onInputStreamRead",
                "$_,$0,$1,$2", int.class, Object.class, byte[].class, int.class);
        insertAfter(ctClass, "read", "([BII)I", readSrc);
    }
}
