/*
 * Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

package com.fuxi.javaagent.plugin.info;

import com.fuxi.javaagent.request.HttpServletRequest;
import com.fuxi.javaagent.tool.OSUtil;
import com.fuxi.javaagent.tool.Reflection;

import java.text.SimpleDateFormat;
import java.util.HashMap;
import java.util.Map;

public class SecurityPolicyInfo extends EventInfo {

    public enum Type {
        COOKIE_HTTP_ONLY(3001),
        START_USER(3002),
        MANAGER_PASSWORD(3003),
        DEFAULT_APP(3004),
        SQL_CONNECTION(3005);

        private int id;

        Type(int id) {
            this.id = id;
        }

        @Override
        public String toString() {
            return String.valueOf(this.id);
        }

    }

    public static final String TYPE_SECURITY_POLICY = "security_policy";

    private Type policy;
    private String message;

    public SecurityPolicyInfo(Type policy, String message, boolean isBlock) {
        this.policy = policy;
        this.message = message;
        setBlock(isBlock);
    }

    @Override
    public String getType() {
        return TYPE_SECURITY_POLICY;
    }

    @Override
    public Map<String, Object> getInfo() {
        Map<String, Object> info = new HashMap<String, Object>();

        info.put("event_type", getType());
        info.put("event_time", new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss").format(System.currentTimeMillis()));
        // policy id
        info.put("policy_id", this.policy.toString());
        // 服务器host name
        info.put("server_hostname", OSUtil.getHostName());
        // 服务器ip
        info.put("nic", OSUtil.getIpAddress());
        // 服务器类型
        info.put("server_type", getCatalinaServerType());
        // 服务器版本
        info.put("server_version", getCatalinaServerVersion());
        // 安全规范检测信息
        info.put("message", message);
        return info;
    }

    public static String getCatalinaServerType() {
        String serverInfo = (String) Reflection.invokeStaticMethod("org.apache.catalina.util.ServerInfo",
                "getServerInfo", new Class[]{});
        return HttpServletRequest.extractType(serverInfo);
    }

    public static String getCatalinaServerVersion() {
        String serverInfo = (String) Reflection.invokeStaticMethod("org.apache.catalina.util.ServerInfo",
                "getServerInfo", new Class[]{});
        return HttpServletRequest.extractNumber(serverInfo);
    }
}
