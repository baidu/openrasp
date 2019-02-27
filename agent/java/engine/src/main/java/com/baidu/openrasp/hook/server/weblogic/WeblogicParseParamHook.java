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

package com.baidu.openrasp.hook.server.weblogic;

import com.baidu.openrasp.hook.server.ServerParamHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

/**
 * @author anyang
 * @Description: TODO
 * @date 2018/8/27 16:40
 */
@HookAnnotation
public class WeblogicParseParamHook extends ServerParamHook {

    @Override
    public boolean isClassMatched(String className) {
        return "weblogic/servlet/internal/ServletRequestImpl$RequestParameters".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass, String src) throws NotFoundException, CannotCompileException {
        insertBefore(ctClass, "parseQueryParams", null, src);
        insertBefore(ctClass, "parseExtraQueryParams", null, src);
    }

}
