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

package com.baidu.openrasp.hook.ssrf.redirect;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.Reflection;

import java.util.HashMap;

/**
 * Created by tyy on 19-11-25.
 */
public abstract class AbstractRedirectHook extends AbstractClassHook {

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "ssrfRedirect";
    }

    public static void checkHttpClientRedirect(HashMap<String, Object> params,
                                               HashMap<String, Object> redirectParams, Object response) {
        params.put("url2", redirectParams.get("url"));
        params.put("hostname2", redirectParams.get("hostname"));
        params.put("port2", redirectParams.get("port"));
        params.put("ip2", redirectParams.get("ip"));
        Object statusLine = Reflection.invokeMethod(response, "getStatusLine", new Class[]{});
        if (statusLine != null) {
            String statusMsg = Reflection.invokeStringMethod(statusLine, "getReasonPhrase", new Class[]{});
            statusMsg = statusMsg == null ? "" : statusMsg;
            params.put("http_message", statusMsg);
            int statusCode = (Integer) Reflection.invokeMethod(statusLine, "getStatusCode", new Class[]{});
            statusCode = statusCode < 0 ? 0 : statusCode;
            params.put("http_status", statusCode);
        }
        HookHandler.doCheck(CheckParameter.Type.SSRF_REDIRECT, params);
    }

    public static void checkUrlConnHttpRedirect(HashMap<String, Object> params,
                                                HashMap<String, Object> redirectParams, Object response) {
        Object statusLine = Reflection.invokeMethod(response, "getStatusLine", new Class[]{});
        String statusMsg = "";
        int statusCode = 0;
        if (statusLine != null) {
            statusMsg = Reflection.invokeStringMethod(statusLine, "getReasonPhrase", new Class[]{});
            statusMsg = statusMsg == null ? "" : statusMsg;
            statusCode = (Integer) Reflection.invokeMethod(statusLine, "getStatusCode", new Class[]{});
            statusCode = statusCode < 0 ? 0 : statusCode;
        }
        checkRedirect(params, redirectParams, statusMsg, statusCode);
    }

    public static void checkRedirect(HashMap<String, Object> params,
                                     HashMap<String, Object> redirectParams, String statusMsg, int statusCode) {
        params.put("url2", redirectParams.get("url"));
        params.put("hostname2", redirectParams.get("hostname"));
        params.put("port2", redirectParams.get("port"));
        params.put("ip2", redirectParams.get("ip"));
        params.put("http_message", statusMsg);
        statusCode = statusCode < 0 ? 0 : statusCode;
        params.put("http_status", statusCode);
        HookHandler.doCheck(CheckParameter.Type.SSRF_REDIRECT, params);
    }

}
