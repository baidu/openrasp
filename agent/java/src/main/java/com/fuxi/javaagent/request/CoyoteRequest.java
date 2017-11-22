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

package com.fuxi.javaagent.request;

import com.fuxi.javaagent.tool.Reflection;

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
