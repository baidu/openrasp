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

package com.baidu.openrasp.plugin.checker;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.plugin.checker.js.JsChecker;
import com.baidu.openrasp.plugin.checker.local.SSRFChecker;
import com.baidu.openrasp.plugin.checker.local.SqlResultChecker;
import com.baidu.openrasp.plugin.checker.local.SqlStatementChecker;
import com.baidu.openrasp.plugin.checker.local.XssChecker;
import com.baidu.openrasp.plugin.checker.policy.server.*;
import com.baidu.openrasp.plugin.checker.policy.SqlConnectionChecker;
import com.baidu.openrasp.request.AbstractRequest;
import com.google.gson.Gson;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by tyy on 3/31/17.
 * 用于转化hook参数和filter所需要的参数
 */
public class CheckParameter {

    public static final HashMap<String, Object> EMPTY_MAP = new HashMap<String, Object>();

    public enum Type {
        // js插件检测
        SQL("sql", new SqlStatementChecker(), 1),
        COMMAND("command", new JsChecker(), 1 << 1),
        DIRECTORY("directory", new JsChecker(), 1 << 2),
        REQUEST("request", new JsChecker(), 1 << 3),
        DUBBOREQUEST("dubboRequest", new JsChecker(), 1 << 4),
        READFILE("readFile", new JsChecker(), 1 << 5),
        WRITEFILE("writeFile", new JsChecker(), 1 << 6),
        FILEUPLOAD("fileUpload", new JsChecker(), 1 << 7),
        RENAME("rename", new JsChecker(), 1 << 8),
        XXE("xxe", new JsChecker(), 1 << 9),
        OGNL("ognl", new JsChecker(), 1 << 10),
        DESERIALIZATION("deserialization", new JsChecker(), 1 << 11),
        //        REFLECTION("reflection", new JsChecker()),
        WEBDAV("webdav", new JsChecker(), 1 << 12),
        INCLUDE("include", new JsChecker(), 1 << 13),
        SSRF("ssrf", new SSRFChecker(), 1 << 14),

        // java本地检测
        SQL_SLOW_QUERY("sqlSlowQuery", new SqlResultChecker(false), 0),
        XSS("xss", new XssChecker(), 0),

        // 安全基线检测
        POLICY_SQL_CONNECTION("sqlConnection", new SqlConnectionChecker(), 0),
        POLICY_SERVER_TOMCAT("tomcatServer", new TomcatSecurityChecker(false), 0),
        POLICY_SERVER_JBOSS("jbossServer", new JBossSecurityChecker(false), 0),
        POLICY_SERVER_JETTY("jettyServer", new JettySecurityChecker(false), 0),
        POLICY_SERVER_RESIN("resinServer", new ResinSecurityChecker(false), 0),
        POLICY_SERVER_WEBSPHERE("websphereServer", new WebsphereSecurityChecker(false), 0),
        POLICY_SERVER_WEBLOGIC("weblogicServer", new WeblogicSecurityChecker(false), 0),
        POLICY_SERVER_WILDFLY("wildflyServer", new WildflySecurityChecker(false), 0);

        String name;
        Checker checker;
        Integer code;

        Type(String name, Checker checker, Integer code) {
            this.name = name;
            this.checker = checker;
            this.code = code;
        }

        public String getName() {
            return name;
        }

        public Checker getChecker() {
            return checker;
        }

        public Integer getCode() {
            return code;
        }

        @Override
        public String toString() {
            return name;
        }
    }

    private final Type type;
    private final Object params;
    private final AbstractRequest request;
    private final long createTime;


    public CheckParameter(Type type, Object params) {
        this.type = type;
        this.params = params;
        this.request = HookHandler.requestCache.get();
        this.createTime = System.currentTimeMillis();
    }

    /**
     * 用于单元测试的构造函数
     */
    public CheckParameter(Type type, Object params, AbstractRequest request) {
        this.type = type;
        this.params = params;
        this.request = request;
        this.createTime = System.currentTimeMillis();
    }

    public Object getParam(String key) {
        return params == null ? null : ((Map) params).get(key);
    }

    public Type getType() {
        return type;
    }

    public Object getParams() {
        return params;
    }

    public AbstractRequest getRequest() {
        return request;
    }

    public long getCreateTime() {
        return createTime;
    }

    @Override
    public String toString() {
        Map<String, Object> obj = new HashMap<String, Object>();
        obj.put("type", type);
        obj.put("params", params);
        return new Gson().toJson(obj);
    }
}
