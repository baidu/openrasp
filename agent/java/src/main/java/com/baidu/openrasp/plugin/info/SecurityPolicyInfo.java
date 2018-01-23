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

package com.baidu.openrasp.plugin.info;

import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.request.HttpServletRequest;
import com.baidu.openrasp.tool.OSUtil;
import com.baidu.openrasp.tool.Reflection;

import java.text.SimpleDateFormat;
import java.util.HashMap;
import java.util.Map;

public class SecurityPolicyInfo extends EventInfo {

    public enum Type {
        COOKIE_HTTP_ONLY(3001),
        START_USER(3002),
        MANAGER_PASSWORD(3003),
        DEFAULT_APP(3004),
        DIRECTORY_LISTING(3005),
        SQL_CONNECTION(3006);


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
        setBlock(isBlock && Config.getConfig().getEnforcePolicy());
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
        // 攻击调用栈
        StackTraceElement[] trace = filter(new Throwable().getStackTrace());
        info.put("stack_trace", stringify(trace));
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
