/**
 * Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
 * <p>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * <p>
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * <p>
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * <p>
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * <p>
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

import java.util.HashMap;
import java.util.Map;

/**
 * Created by tyy on 3/31/17.
 * 用于转化hook参数和filter所需要的参数
 */
public class CheckParameter {

    public enum Type {
        SQL("sql"),
        COMMAND("command"),
        DIRECTORY("directory"),
        REQUEST("request"),
        READFILE("readFile"),
        WRITEFILE("writeFile"),
        FILEUPLOAD("fileUpload"),
        XXE("xxe"),
        OGNL("ognl"),
        DESERIALIZATION("deserialization"),
        REFLECTION("reflection"),
        WEBDAV("webdav"),
        INCLUDE("include");

        String normalName;

        Type(String name) {
            normalName = name;
        }

        @Override
        public String toString() {
            return normalName;
        }
    }

    private final Type type;
    private final Map<String, Object> params;
    private final AbstractRequest request;
    private final long createTime;


    public CheckParameter(Type type, Map<String, Object> params, AbstractRequest request) {
        this.type = type;
        this.params = params;
        this.request = request;
        this.createTime = System.currentTimeMillis();
    }

    public Type getType() {
        return type;
    }

    public Map<String, Object> getParams() {
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
