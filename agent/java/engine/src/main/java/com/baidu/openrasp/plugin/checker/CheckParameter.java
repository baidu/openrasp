/*
 * Copyright 2017-2020 Baidu Inc.
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
import com.baidu.openrasp.plugin.checker.local.SqlResultChecker;
import com.baidu.openrasp.plugin.checker.local.XssChecker;
import com.baidu.openrasp.plugin.checker.policy.LogChecker;
import com.baidu.openrasp.plugin.checker.policy.MongoConnectionChecker;
import com.baidu.openrasp.plugin.checker.policy.SqlConnectionChecker;
import com.baidu.openrasp.plugin.checker.policy.server.*;
import com.baidu.openrasp.plugin.checker.v8.V8AttackChecker;
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
        SQL("sql", new V8AttackChecker(), 1),
        COMMAND("command", new V8AttackChecker(), 1 << 1),
        DIRECTORY("directory", new V8AttackChecker(), 1 << 2),
        REQUEST("request", new V8AttackChecker(), 1 << 3),
        READFILE("readFile", new V8AttackChecker(), 1 << 5),
        WRITEFILE("writeFile", new V8AttackChecker(), 1 << 6),
        FILEUPLOAD("fileUpload", new V8AttackChecker(), 1 << 7),
        RENAME("rename", new V8AttackChecker(), 1 << 8),
        XXE("xxe", new V8AttackChecker(), 1 << 9),
        OGNL("ognl", new V8AttackChecker(), 1 << 10),
        DESERIALIZATION("deserialization", new V8AttackChecker(), 1 << 11),
        WEBDAV("webdav", new V8AttackChecker(), 1 << 12),
        INCLUDE("include", new V8AttackChecker(), 1 << 13),
        SSRF("ssrf", new V8AttackChecker(), 1 << 14),
        SQL_EXCEPTION("sql_exception", new V8AttackChecker(), 1 << 15),
        REQUESTEND("requestEnd", new V8AttackChecker(), 1 << 17),
        DELETEFILE("deleteFile", new V8AttackChecker(), 1 << 18),
        MONGO("mongodb", new V8AttackChecker(), 1 << 19),
        LOADLIBRARY("loadLibrary", new V8AttackChecker(), 1 << 20),
        SSRF_REDIRECT("ssrfRedirect", new V8AttackChecker(), 1 << 21),
        RESPONSE("response", new V8AttackChecker(false), 1 << 23),
        LINK("link", new V8AttackChecker(), 1 << 24),

        // java本地检测
        XSS_USERINPUT("xss_userinput", new XssChecker(), 1 << 16),
        SQL_SLOW_QUERY("sqlSlowQuery", new SqlResultChecker(false), 0),

        // 安全基线检测
        POLICY_LOG("log", new LogChecker(false), 1 << 22),
        POLICY_MONGO_CONNECTION("mongoConnection", new MongoConnectionChecker(false), 0),
        POLICY_SQL_CONNECTION("sqlConnection", new SqlConnectionChecker(false), 0),
        POLICY_SERVER_TOMCAT("tomcatServer", new TomcatSecurityChecker(false), 0),
        POLICY_SERVER_JBOSS("jbossServer", new JBossSecurityChecker(false), 0),
        POLICY_SERVER_JBOSSEAP("jbossEAPServer", new JBossEAPSecurityChecker(false), 0),
        POLICY_SERVER_JETTY("jettyServer", new JettySecurityChecker(false), 0),
        POLICY_SERVER_RESIN("resinServer", new ResinSecurityChecker(false), 0),
        POLICY_SERVER_WEBSPHERE("websphereServer", new WebsphereSecurityChecker(false), 0),
        POLICY_SERVER_WEBLOGIC("weblogicServer", new WeblogicSecurityChecker(false), 0),
        POLICY_SERVER_WILDFLY("wildflyServer", new WildflySecurityChecker(false), 0),
        POLICY_SERVER_TONGWEB("tongwebServer", new TongwebSecurityChecker(false), 0),
        POLICY_SERVER_BES("bes", new BESSecurityChecker(false), 0);

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
    private final Map params;
    private final AbstractRequest request;
    private final long createTime;


    public CheckParameter(Type type, Map params) {
        this.type = type;
        this.params = params;
        this.request = HookHandler.requestCache.get();
        this.createTime = System.currentTimeMillis();
    }

    /**
     * 用于单元测试的构造函数
     */
    public CheckParameter(Type type, Map params, AbstractRequest request) {
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

    public Map getParams() {
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
