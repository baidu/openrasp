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

package com.baidu.openrasp.hook.ssrf;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.js.engine.JSContext;
import com.baidu.openrasp.plugin.js.engine.JSContextFactory;
import org.mozilla.javascript.Scriptable;

/**
 * Created by tyy on 17-12-9.
 *
 * SSRF hook点基类
 */
public abstract class AbstractSSRFHook extends AbstractClassHook {

    @Override
    public String getType() {
        return "ssrf";
    }

    protected static void checkHttpUrl(String url, String hostName,String function) {
        JSContext cx = JSContextFactory.enterAndInitContext();
        Scriptable params = cx.newObject(cx.getScope());
        params.put("url", params, url);
        params.put("hostname", params, hostName);
        params.put("function", params, function);
        HookHandler.doCheck(CheckParameter.Type.SSRF, params);
    }

}
