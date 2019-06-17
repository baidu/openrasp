/*
 * Copyright 2017-2019 Baidu Inc.
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

import java.net.Inet4Address;
import java.net.InetAddress;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedList;

/**
 * Created by tyy on 17-12-9.
 * <p>
 * SSRF hook点基类
 */
public abstract class AbstractSSRFHook extends AbstractClassHook {

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "ssrf";
    }

    /**
     * ssrf 检测的入口
     *
     * @param url      http 请求的 url
     * @param hostName http 请求的 host
     * @param function http 请求的方式
     */
    protected static void checkHttpUrl(String url, String hostName, String port, String function) {
        HashMap<String, Object> params = new HashMap<String, Object>();
        params.put("url", url);
        params.put("hostname", hostName);
        params.put("function", function);
        params.put("port", port);
        LinkedList<String> ip = new LinkedList<String>();
        try {
            InetAddress[] addresses = InetAddress.getAllByName(hostName);
            for (InetAddress address : addresses) {
                if (address != null && address instanceof Inet4Address) {
                    ip.add(address.getHostAddress());
                }
            }
        } catch (Throwable t) {
            // ignore
        }
        Collections.sort(ip);
        params.put("ip", ip);
        HookHandler.doCheck(CheckParameter.Type.SSRF, params);
    }

}
