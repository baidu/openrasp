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

package com.baidu.openrasp.request;

import java.util.Enumeration;
import java.util.HashMap;
import java.util.Map;

/**
 * Created by tyy on 18-5-11.
 *
 * 空请求实体，用于匹配非请求检测
 */
public class EmptyRequest extends AbstractRequest {

    @Override
    public String getLocalAddr() {
        return null;
    }

    @Override
    public String getMethod() {
        return null;
    }

    @Override
    public String getProtocol() {
        return null;
    }

    @Override
    public String getAuthType() {
        return null;
    }

    @Override
    public String getContextPath() {
        return null;
    }

    @Override
    public String getRemoteAddr() {
        return null;
    }

    @Override
    public String getRequestURI() {
        return null;
    }

    @Override
    public StringBuffer getRequestURL() {
        return null;
    }

    @Override
    public String getServerName() {
        return null;
    }

    @Override
    public String getParameter(String key) {
        return null;
    }

    @Override
    public Enumeration<String> getParameterNames() {
        return null;
    }

    @Override
    public Map<String, String[]> getParameterMap() {
        return null;
    }

    @Override
    public String getHeader(String key) {
        return null;
    }

    @Override
    public Enumeration<String> getHeaderNames() {
        return null;
    }

    @Override
    public String getQueryString() {
        return null;
    }

    @Override
    public Map<String, String> getServerContext() {
        HashMap<String, String> serverContext = new HashMap<String, String>(4);
        serverContext.put("language", "java");
        return serverContext;
    }

    @Override
    public String getAppBasePath() {
        return null;
    }

    @Override
    public String getClinetIp() {
        return null;
    }

    @Override
    public String getContentType() {
        return null;
    }

    @Override
    public String getCharacterEncoding() {
        return null;
    }
}
