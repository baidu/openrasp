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

package com.baidu.openrasp.hook.server;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.AbstractClassHook;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * Created by tyy on 18-2-8.
 *
 * 服务器请求 hook 点基类
 */
public abstract class ServerRequestHook extends AbstractClassHook {

    /**
     * (none-javadoc)
     * tyy
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "request";
    }

    /**
     * catalina 请求 hook 点检测入口
     *
     * @param filter   ApplicationFilterChain 实例本身
     * @param request  请求实体
     * @param response 响应实体
     */
    public static void checkRequest(Object filter, Object request, Object response) {
        HookHandler.checkFilterRequest(filter, request, response);
    }

}
