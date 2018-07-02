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

package com.baidu.openrasp.request;

import com.baidu.openrasp.tool.Reflection;

import java.util.Enumeration;
import java.util.HashMap;
import java.util.Map;

/**
 * Created by zhuming01 on 6/23/17.
 * All rights reserved
 * org.apache.coyote.Request类请求的的格式化接口
 */
public class CoyoteRequest extends AbstractRequest {

    /**
     * 请求实体
     *
     * @param request 类型为org.apache.coyote.Request的请求实体
     */
    public CoyoteRequest(Object request) {
        super(request);
    }

    /**
     * 把 message bytes 字节类型信息数据转化成字符串类型数据
     *
     * @param messageBytes 字节类型的信息数据
     * @return 转化之后的字符串类型的信息数据
     */
    private String mb2string(Object messageBytes) {
        return Reflection.invokeStringMethod(messageBytes, "toString", EMPTY_CLASS);
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getLocalAddr()
     */
    @Override
    public String getLocalAddr() {
        return mb2string(Reflection.invokeMethod(request, "localAddr", EMPTY_CLASS));
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getMethod()
     */
    @Override
    public String getMethod() {
        return mb2string(Reflection.invokeMethod(request, "method", EMPTY_CLASS));
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getProtocol()
     */
    @Override
    public String getProtocol() {
        return Reflection.invokeStringMethod(request, "protocol", EMPTY_CLASS);
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getAuthType()
     */
    @Override
    public String getAuthType() {
        return mb2string(Reflection.invokeMethod(request, "getAuthType", EMPTY_CLASS));
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getContextPath()
     */
    @Override
    public String getContextPath() {
        return null;
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getRemoteAddr()
     */
    @Override
    public String getRemoteAddr() {
        return mb2string(Reflection.invokeMethod(request, "remoteAddr", EMPTY_CLASS));
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getRequestURI()
     */
    @Override
    public String getRequestURI() {
        return mb2string(Reflection.invokeMethod(request, "requestURI", EMPTY_CLASS));
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getRequestURL()
     */
    @Override
    public StringBuffer getRequestURL() {
        StringBuffer sb = new StringBuffer();

        String scheme = mb2string(Reflection.invokeMethod(request, "scheme", EMPTY_CLASS));
        if (scheme != null) {
            sb.append(scheme).append("://");
        }

        String host = getServerName();
        if (host != null) {
            sb.append(host);
        }

        String port = getServerPort();
        if (port != null) {
            sb.append(":").append(port);
        }

        String uri = getRequestURI();
        if (uri != null) {
            sb.append(uri);
        }

        return sb;
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getServerName()
     */
    @Override
    public String getServerName() {
        return mb2string(Reflection.invokeMethod(request, "serverName", EMPTY_CLASS));
    }

    /**
     * 获取本服务的端口号
     *
     * @return 端口号
     */
    private String getServerPort() {
        Object port = Reflection.invokeMethod(request, "getServerPort", EMPTY_CLASS);
        return port != null ? port.toString() : null;
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getParameter(String)
     */
    @Override
    public String getParameter(String key) {
        Object parameters = Reflection.invokeMethod(request, "getParameters", EMPTY_CLASS);
        if (parameters == null)
            return null;

        return Reflection.invokeStringMethod(parameters, "getParameter", STRING_CLASS, key);
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getParameterNames()
     */
    @Override
    public Enumeration<String> getParameterNames() {
        Object parameters = Reflection.invokeMethod(request, "getParameters", EMPTY_CLASS);
        if (parameters == null)
            return null;

        Object names = Reflection.invokeMethod(parameters, "getParameterNames", EMPTY_CLASS);
        if (names == null)
            return null;

        return (Enumeration<String>) names;
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getParameterMap()
     */
    @Override
    public Map<String, String[]> getParameterMap() {
        Enumeration<String> names = getParameterNames();
        if (names == null) {
            return null;
        }
        Map<String, String[]> paramMap = new HashMap<String, String[]>();
        while (names.hasMoreElements()) {
            String key = names.nextElement();
            String[] values = (String[]) Reflection.invokeMethod(request, "getParameterValues", STRING_CLASS, key);
            paramMap.put(key, values);
        }
        return paramMap;
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getHeader(String)
     */
    @Override
    public String getHeader(String key) {
        return Reflection.invokeStringMethod(request, "getHeader", STRING_CLASS, key);
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getHeaderNames()
     */
    @Override
    public Enumeration<String> getHeaderNames() {
        return null;
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getQueryString()
     */
    @Override
    public String getQueryString() {
        return mb2string(Reflection.invokeMethod(request, "queryString", EMPTY_CLASS));
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getServerContext()
     */
    @Override
    public Map<String, String> getServerContext() {
        return null;
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getAppBasePath()
     */
    @Override
    public String getAppBasePath() {
        return null;
    }
}
