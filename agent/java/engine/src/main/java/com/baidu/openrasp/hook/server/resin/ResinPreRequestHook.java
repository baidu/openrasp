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

import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.hook.server.ServerPreRequestHook;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

/**
 * Created by tyy on 18-2-11.
 *
 * resin 服务器请求预处理 hook 点
 */
public class ResinPreRequestHook extends ServerPreRequestHook {

    /**
     * (none-javadoc)
     *
     * @see AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "com/caucho/server/http/HttpRequest".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass, String src) throws NotFoundException, CannotCompileException {
        insertBefore(ctClass, "handleRequest", "()Z", src);
        insertBefore(ctClass, "handleResume", "()Z", src);
    }
}
