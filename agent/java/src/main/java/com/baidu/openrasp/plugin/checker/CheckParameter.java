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

package com.baidu.openrasp.plugin.checker;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.plugin.checker.js.JsChecker;
import com.baidu.openrasp.plugin.checker.local.SSRFChecker;
import com.baidu.openrasp.plugin.checker.local.SqlResultChecker;
import com.baidu.openrasp.plugin.checker.local.SqlStatementChecker;
import com.baidu.openrasp.plugin.checker.policy.SqlConnectionChecker;
import com.baidu.openrasp.plugin.checker.policy.TomcatSecurityChecker;
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
        SQL("sql", new SqlStatementChecker()),
        COMMAND("command", new JsChecker()),
        DIRECTORY("directory", new JsChecker()),
        REQUEST("request", new JsChecker()),
        READFILE("readFile", new JsChecker()),
        WRITEFILE("writeFile", new JsChecker()),
        FILEUPLOAD("fileUpload", new JsChecker()),
        XXE("xxe", new JsChecker()),
        OGNL("ognl", new JsChecker()),
        DESERIALIZATION("deserialization", new JsChecker()),
//        REFLECTION("reflection", new JsChecker()),
        WEBDAV("webdav", new JsChecker()),
        INCLUDE("include", new JsChecker()),
        SSRF("ssrf", new SSRFChecker()),

        // java本地检测
        SQL_SLOW_QUERY("sqlSlowQuery", new SqlResultChecker(false)),

        // 安全基线检测
        POLICY_SQL_CONNECTION("sqlConnection", new SqlConnectionChecker()),
        POLICY_TOMCAT_START("tomcatStart", new TomcatSecurityChecker());

        String name;
        Checker checker;

        Type(String name, Checker checker) {
            this.name = name;
            this.checker = checker;
        }

        public String getName() {
            return name;
        }

        public Checker getChecker() {
            return checker;
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
