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

package com.fuxi.javaagent.plugin.checker;

import com.fuxi.javaagent.HookHandler;
import com.fuxi.javaagent.plugin.checker.js.JsChecker;
import com.fuxi.javaagent.plugin.checker.local.SqlResultChecker;
import com.fuxi.javaagent.plugin.checker.policy.SqlConnectionChecker;
import com.fuxi.javaagent.plugin.checker.policy.TomcatSecurityChecker;
import com.fuxi.javaagent.request.AbstractRequest;
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
        SQL("sql", new JsChecker()),
        COMMAND("command", new JsChecker()),
        DIRECTORY("directory", new JsChecker()),
        REQUEST("request", new JsChecker()),
        READFILE("readFile", new JsChecker()),
        WRITEFILE("writeFile", new JsChecker()),
        FILEUPLOAD("fileUpload", new JsChecker()),
        XXE("xxe", new JsChecker()),
        OGNL("ognl", new JsChecker()),
        DESERIALIZATION("deserialization", new JsChecker()),
        REFLECTION("reflection", new JsChecker()),
        WEBDAV("webdav", new JsChecker()),
        INCLUDE("include", new JsChecker()),
        SSRF("ssrf", new JsChecker()),

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
