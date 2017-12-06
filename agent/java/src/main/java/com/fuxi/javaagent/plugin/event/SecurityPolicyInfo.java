package com.fuxi.javaagent.plugin.event;

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

        private Type(int id) {
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

    public SecurityPolicyInfo(Type policy, String message) {
        this.policy = policy;
        this.message = message;
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
