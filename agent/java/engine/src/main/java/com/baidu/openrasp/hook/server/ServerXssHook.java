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
import com.baidu.openrasp.response.HttpServletResponse;

/**
 * @author anyang
 * @Description: xss检测基类
 * @date 2018/8/15 15:37
 */
public abstract class ServerXssHook extends AbstractClassHook {
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
}
