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
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.response.HttpServletResponse;

import java.util.HashMap;
import com.baidu.openrasp.tool.Sampler;

/**
 * @author anyang
 * @Description: xss检测基类
 * @date 2018/8/15 15:37
 */
public abstract class ServerResponseBodyHook extends AbstractClassHook {
    private static Sampler sampler = new Sampler();

    @Override
    public String getType() {
        return "xss";
    }

    protected static boolean isCheckXss() {
        if (HookHandler.requestCache.get() != null && HookHandler.responseCache.get() != null) {
            String contentType = HookHandler.responseCache.get().getContentType();
            return contentType == null || contentType.startsWith(HttpServletResponse.CONTENT_TYPE_HTML_VALUE);
        }
        return false;
    }

    protected static boolean isCheckSensitive() {
        Config config = Config.getConfig();
        if (config.getResponseSamplerInterval() <= 0 || config.getResponseSamplerBurst() <= 0) {
            return false;
        }
        if (HookHandler.responseCache.get() != null) {
            String contentType = HookHandler.responseCache.get().getContentType();
            if (contentType != null && (contentType.contains("video") || contentType.contains("audio")
                    || contentType.contains("image"))) {
                return false;
            }
        }
        sampler.update(config.getResponseSamplerInterval(), config.getResponseSamplerBurst());
        // 限速检测
        return sampler.check();
    }

    protected static void checkBody(HashMap<String, Object> params, boolean isCheckXss, boolean isCheckSensitive) {
        if (isCheckXss) {
            HookHandler.doCheck(CheckParameter.Type.XSS_USERINPUT, params);
        }
        if (isCheckSensitive) {
            params.remove("buffer");
            params.remove("encoding");
            params.remove("content_length");
            HookHandler.doCheck(CheckParameter.Type.RESPONSE, params);
        }
    }
}
