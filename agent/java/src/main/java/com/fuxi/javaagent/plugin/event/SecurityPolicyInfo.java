package com.fuxi.javaagent.plugin.event;

import com.fuxi.javaagent.request.HttpServletRequest;
import com.fuxi.javaagent.tool.OSUtil;
import com.fuxi.javaagent.tool.Reflection;

import java.text.SimpleDateFormat;
import java.util.HashMap;
import java.util.Map;

public class SecurityPolicyInfo extends EventInfo {

    public static final String TYPE_SECURITY_POLICY = "security_policy";

    private String message;

    public SecurityPolicyInfo(String message) {
        this.message = message;
    }

    @Override
    public String getType() {
        return TYPE_SECURITY_POLICY;
    }

    @Override
    public Map<String, Object> getInfo() {
        Map<String, Object> info = new HashMap<String, Object>();
        String serverInfo = (String) Reflection.invokeStaticMethod("org.apache.catalina.util.ServerInfo",
                "getServerInfo", new Class[]{});

        info.put("event_type", getType());
        info.put("event_time", new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss").format(System.currentTimeMillis()));
        // 服务器host name
        info.put("server_hostname", OSUtil.getHostName());
        // 服务器ip
        info.put("nic", OSUtil.getIpAddress());
        // 服务器类型
        info.put("server_type", HttpServletRequest.extractType(serverInfo));
        // 服务器版本
        info.put("server_version", HttpServletRequest.extractNumber(serverInfo));
        // 安全规范检测信息
        info.put("message", message);
        return info;
    }
}
