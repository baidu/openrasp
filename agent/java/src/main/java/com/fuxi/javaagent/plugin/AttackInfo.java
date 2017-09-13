/**
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

package com.fuxi.javaagent.plugin;

import com.fuxi.javaagent.request.AbstractRequest;
import com.google.gson.Gson;

import java.sql.Timestamp;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

/**
 * Created by zhuming01 on 7/11/17.
 * All rights reserved
 * 攻击信息类，主要用于报警
 *
 * @see <a href="https://rasp.baidu.com/doc/setup/log/main.html">document</a>
 */
public class AttackInfo {
    private CheckParameter parameter;
    private CheckResult result;
    private String json;

    AttackInfo(CheckParameter parameter, CheckResult result) {
        this.parameter = parameter;
        this.result = result;
        this.json = null;
    }

    /**
     * 转换攻击日志为json格式
     *
     * @return json格式日志
     */
    String toJson() {
        if (json == null) {
            Map<String, Object> info = getInfo();
            json = new Gson().toJson(info);
        }
        return json;
    }

    /**
     * 整理攻击请求的信息
     *
     * @return 攻击信息
     */
    private Map<String, Object> getInfo() {
        Map<String, Object> info = new HashMap<String, Object>();
        AbstractRequest request = parameter.getRequest();
        Timestamp createTime = new Timestamp(parameter.getCreateTime());

        // 攻击时间
        info.put("attack_time", new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss").format(createTime));
        // 攻击类型
        info.put("attack_type", parameter.getType());
        // 攻击参数
        info.put("attack_params", parameter.getParams());
        // 攻击调用栈
        StackTraceElement[] trace = filter(new Throwable().getStackTrace());
        info.put("stack_trace", stringify(trace));
        // 检测插件
        info.put("plugin_name", result.getPluginName());
        // 插件消息
        info.put("plugin_message", result.getMessage());
        // 是否拦截
        info.put("intercept_state", result.getResult());

        if (request != null) {
            // 攻击来源IP
            info.put("attack_source", request.getRemoteAddr());
            // 被攻击目标域名
            info.put("target", request.getServerName());
            // 被攻击目标IP
            info.put("server_ip", request.getLocalAddr());
            // 被攻击目标服务器类型和版本
            Map<String, String> serverInfo = request.getServerContext();
            info.put("server_type", serverInfo != null ? serverInfo.get("server") : null);
            info.put("server_version", serverInfo != null ? serverInfo.get("version") : null);
            // 被攻击URL
            StringBuffer requestURL = request.getRequestURL();
            String queryString = request.getQueryString();
            info.put("url", requestURL == null ? "" : (queryString != null ? requestURL + "?" + queryString : requestURL));
            // 请求体
            byte[] requestBody = request.getBody();
            if(requestBody != null) {
                info.put("body", new String(requestBody));
            }
            // 被攻击PATH
            info.put("path", request.getRequestURI());
            // 用户代理
            info.put("user_agent", request.getHeader("User-Agent"));
            // 攻击的 Referrer 头
            String referer = request.getHeader("Referer");
            if(referer != null){
                info.put("referer", referer);
            }
        }

        return info;
    }

    private StackTraceElement[] filter(StackTraceElement[] trace) {
        int i = 0;
        // 去除插件本身调用栈
        while (trace[i].getClassName().startsWith("com.fuxi.javaagent")) {
            i++;
        }
        // 只显示最近的５层调用栈
        return Arrays.copyOfRange(trace, i, Math.min(i + 5, trace.length));
    }

    private String stringify(StackTraceElement[] trace) {
        StringBuilder ret = new StringBuilder();
        for (int i = 0; i < trace.length - 1; i++) {
            ret.append(trace[i].toString());
            ret.append("\n");
        }
        ret.append(trace[trace.length - 1].toString());
        return ret.toString();
    }

    @Override
    public String toString() {
        if (json == null) {
            Map<String, Object> info = getInfo();
            json = new Gson().toJson(info);
        }
        return json;
    }
}
