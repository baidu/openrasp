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
import com.baidu.openrasp.plugin.checker.CheckParameter;

import java.util.HashMap;

/**
 * @description: 服务器的request end hook点
 * @author: anyang
 * @create: 2019/05/31 17:22
 */
public abstract class ServerRequestEndHook extends AbstractClassHook {

    public ServerRequestEndHook() {
        isNecessary = true;
    }

    @Override
    public String getType() {
        return "requestEnd";
    }

    public static void checkRequestEnd() {
        if (HookHandler.enableEnd.get()) {
            HookHandler.doCheck(CheckParameter.Type.REQUESTEND, new HashMap<String, Object>());
            HookHandler.enableEnd.set(false);
        }
    }

}
