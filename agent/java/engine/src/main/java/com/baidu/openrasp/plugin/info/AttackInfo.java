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

package com.baidu.openrasp.plugin.info;

import com.baidu.openrasp.cloud.model.CloudCacheModel;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.request.AbstractRequest;
import com.baidu.openrasp.tool.OSUtil;
import com.baidu.openrasp.tool.decompile.Decompiler;
import com.baidu.openrasp.tool.model.ApplicationModel;

import java.sql.Timestamp;
import java.text.SimpleDateFormat;
import java.util.HashMap;
import java.util.Map;

/**
 * Created by zhuming01 on 7/11/17.
 * All rights reserved
 * 攻击信息类，主要用于报警
 *
 * @see <a href="https://rasp.baidu.com/doc/setup/log/main.html">document</a>
 */
public class AttackInfo extends EventInfo {
    public static final String TYPE_ATTACK = "attack";
    public static final String DEFAULT_LOCAL_PLUGIN_NAME = "java_builtin_plugin";

    public static final int DEFAULT_CONFIDENCE_VALUE = 100;

    private CheckParameter parameter;
    private String pluginName;
    private String message;
    private String action;
    private String algorithm;
    private int confidence;

    public static AttackInfo createLocalAttackInfo(CheckParameter parameter, String action,
                                                   String message, String algorithm) {
        return new AttackInfo(parameter, action, message, DEFAULT_LOCAL_PLUGIN_NAME, algorithm);
    }

    public static AttackInfo createLocalAttackInfo(CheckParameter parameter, String action,
                                                   String message, String algorithm, int confidence) {
        return new AttackInfo(parameter, action, message, DEFAULT_LOCAL_PLUGIN_NAME, algorithm, confidence);
    }

    public AttackInfo(CheckParameter parameter, String action, String message,
                      String pluginName, String algorithm) {
        this(parameter, action, message, pluginName, algorithm, DEFAULT_CONFIDENCE_VALUE);
    }

    public AttackInfo(CheckParameter parameter, String action, String message,
                      String pluginName, String algorithm, int confidence) {
        this.message = message;
        this.pluginName = pluginName;
        this.action = action;
        this.confidence = confidence;
        this.parameter = parameter;
        this.algorithm = algorithm;
        setBlock(CHECK_ACTION_BLOCK.equals(action));
    }

    /**
     * 整理攻击请求的信息
     *
     * @return 攻击信息
     */
    @Override
    public Map<String, Object> getInfo() {
        Map<String, Object> info = new HashMap<String, Object>();
        AbstractRequest request = parameter.getRequest();
        Timestamp createTime = new Timestamp(parameter.getCreateTime());

        info.put("event_type", getType());
        // 攻击时间
        info.put("event_time", new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ssZ").format(createTime));
        // 服务器host name
        info.put("server_hostname", OSUtil.getHostName());
        // 攻击类型
        info.put("attack_type", parameter.getType().toString());
        // 攻击参数
        info.put("attack_params", parameter.getParams());
        // 攻击调用栈
        StackTraceElement[] trace = filter(new Throwable().getStackTrace());
        info.put("stack_trace", stringify(trace) != null ? stringify(trace).trim() : null);
        // 检测插件
        info.put("plugin_name", this.pluginName);
        // 插件消息
        info.put("plugin_message", this.message);
        // 插件置信度
        info.put("plugin_confidence", this.confidence);
        // 是否拦截
        info.put("intercept_state", this.action);
        // 检测算法
        info.put("plugin_algorithm", this.algorithm);
        if (Config.getConfig().getCloudSwitch()) {
            // raspId
            info.put("rasp_id", CloudCacheModel.getInstance().getRaspId());
            // appId
            info.put("app_id", Config.getConfig().getCloudAppId());
        }
        if (request != null) {
            // 请求ID
            info.put("request_id", request.getRequestId());
            // 攻击来源IP
            info.put("attack_source", request.getRemoteAddr());
            // 攻击真实IP
            info.put("client_ip", request.getClinetIp());
            // 服务器ip
            info.put("server_nic", OSUtil.getIpAddress());
            // 被攻击目标域名
            info.put("target", request.getServerName());
            // 被攻击目标IP
            info.put("server_ip", request.getLocalAddr());
            // 被攻击目标服务器类型和版本
            info.put("server_type", ApplicationModel.getServerName());
            info.put("server_version", ApplicationModel.getVersion());
            //请求header
            info.put("header", getRequestHeader(request));
            // 被攻击URL
            StringBuffer requestURL = request.getRequestURL();
            String queryString = request.getQueryString();
            info.put("url", requestURL == null ? "" : (queryString != null ? requestURL + "?" + queryString : requestURL));
            // 请求体
            byte[] requestBody = request.getBody();
            if (requestBody != null) {
                info.put("body", new String(requestBody));
            }
            // 被攻击PATH
            info.put("path", request.getRequestURI());
            //请求方法
            String method = request.getMethod();
            info.put("request_method", method != null ? method.toLowerCase() : null);
            //Java反编译开关打开时，启用
            if (Config.getConfig().getDecompileEnable() && checkTomcatVersion()) {
                info.put("source_code", Decompiler.getAlarmPoint(trace));
            }
        }

        return info;
    }

    private boolean checkTomcatVersion() {
        String javaVersion = System.getProperty("java.version");
        return javaVersion != null && (javaVersion.startsWith("1.7")
                || javaVersion.startsWith("1.8"));
    }

    @Override
    public String getType() {
        return TYPE_ATTACK;
    }

    public String getPluginName() {
        return pluginName;
    }

    public String getAction() {
        return action;
    }

    public String getMessage() {
        return message;
    }

    public int getConfidence() {
        return confidence;
    }

    public void setMessage(String message) {
        this.message = message;
    }
}
