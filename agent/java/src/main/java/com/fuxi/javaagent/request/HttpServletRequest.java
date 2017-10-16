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
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Created by zhuming01 on 6/15/17.
 * All rights reserved
 * javax.servlet.http.HttpServletRequest类请求的统一格式接口
 */
public final class HttpServletRequest extends AbstractRequest {
    private static final Pattern PATTERN = Pattern.compile("\\d+(\\.\\d+)*");

    /**
     * 请求实体
     *
     * @param request 类型为javax.servlet.http.HttpServletRequest的请求实体
     */
    public HttpServletRequest(Object request) {
        super(request);
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getLocalAddr()
     */
    @Override
    public String getLocalAddr() {
        return Reflection.invokeStringMethod(request, "getLocalAddr", EMPTY_CLASS);
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getMethod()
     */
    @Override
    public String getMethod() {
        return Reflection.invokeStringMethod(request, "getMethod", EMPTY_CLASS);
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getProtocol()
     */
    @Override
    public String getProtocol() {
        return Reflection.invokeStringMethod(request, "getProtocol", EMPTY_CLASS);
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getAuthType()
     */
    @Override
    public String getAuthType() {
        return Reflection.invokeStringMethod(request, "getAuthType", EMPTY_CLASS);
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getContextPath()
     */
    @Override
    public String getContextPath() {
        return Reflection.invokeStringMethod(request, "getContextPath", EMPTY_CLASS);
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getRemoteAddr()
     */
    @Override
    public String getRemoteAddr() {
        return Reflection.invokeStringMethod(request, "getRemoteAddr", EMPTY_CLASS);
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getRequestURI()
     */
    @Override
    public String getRequestURI() {
        return Reflection.invokeStringMethod(request, "getRequestURI", EMPTY_CLASS);
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getRequestURL()
     */
    @Override
    public StringBuffer getRequestURL() {
        Object ret = Reflection.invokeMethod(request, "getRequestURL", EMPTY_CLASS);
        return ret != null ? (StringBuffer) ret : null;
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getServerName()
     */
    @Override
    public String getServerName() {
        return Reflection.invokeStringMethod(request, "getServerName", EMPTY_CLASS);
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getParameter(String)
     */
    @Override
    public String getParameter(String key) {
        return Reflection.invokeStringMethod(request, "getParameter", STRING_CLASS, key);
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getParameterNames()
     */
    @Override
    public Enumeration<String> getParameterNames() {
        Object ret = Reflection.invokeMethod(request, "getParameterNames", EMPTY_CLASS);
        return ret != null ? (Enumeration) ret : null;
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getParameterMap()
     */
    @Override
    public Map<String, String[]> getParameterMap() {
        Object ret = Reflection.invokeMethod(request, "getParameterMap", EMPTY_CLASS);
        return ret != null ? (Map<String, String[]>) ret : null;
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
        Object ret = Reflection.invokeMethod(request, "getHeaderNames", EMPTY_CLASS);
        return ret != null ? (Enumeration<String>) ret : null;
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getQueryString()
     */
    @Override
    public String getQueryString() {
        return Reflection.invokeStringMethod(request, "getQueryString", EMPTY_CLASS);
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getServerContext()
     */
    @Override
    public Map<String, String> getServerContext() {
        Object session = Reflection.invokeMethod(request, "getSession", new Class[]{boolean.class}, true);
        Object servletContext = Reflection.invokeMethod(session, "getServletContext", EMPTY_CLASS);
        String serverInfo = Reflection.invokeStringMethod(servletContext, "getServerInfo", EMPTY_CLASS);

        Map<String, String> ret = new HashMap<String, String>();
        // TODO more reliable
        ret.put("server", extractType(serverInfo));
        ret.put("version", extractNumber(serverInfo));
        ret.put("os", System.getProperty("os.name"));
        ret.put("language", "java");
        ret.put("__raw", serverInfo);
        return ret;
    }

    /**
     * 根据服务器信息获取服务器类型
     *
     * @param serverInfo 服务器信息
     * @return 服务器类型
     */
    public static String extractType(String serverInfo) {
        if (serverInfo == null) {
            return null;
        }
        serverInfo = serverInfo.toLowerCase();
        if (serverInfo.contains("tomcat")) return "Tomcat";
        if (serverInfo.contains("jboss")) return "JBoss";
        if (serverInfo.contains("jetty")) return "Jetty";
        return serverInfo;
    }

    /**
     * 获取服务器版本号
     *
     * @param serverInfo 服务器信息
     * @return 服务器版本号
     */
    public static String extractNumber(String serverInfo) {
        if (serverInfo == null) {
            return null;
        }
        Matcher m = PATTERN.matcher(serverInfo);
        return m.find() ? m.group(0) : "";
    }

    /**
     * 增加session条目
     *
     * @param key   键
     * @param value 值
     */
    public void setSessionAttribute(String key, String value) {
        Object session = Reflection.invokeMethod(request, "getSession", EMPTY_CLASS);
        Reflection.invokeMethod(session, "setAttribute", new Class[]{String.class, Object.class}, key, value);
    }

    /**
     * 获取指定键对应session
     *
     * @param key 键
     */
    public Object getSessionAttribute(String key) {
        Object session = Reflection.invokeMethod(request, "getSession", EMPTY_CLASS);
        Object value = Reflection.invokeMethod(session, "getAttribute", new Class[]{String.class}, key);
        return value;
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getAppBasePath()
     */
    @Override
    public String getAppBasePath() {
        try {
            Object session = Reflection.invokeMethod(request, "getSession", EMPTY_CLASS);
            Object servletContext = Reflection.invokeMethod(session, "getServletContext", EMPTY_CLASS);
            Object realPath = Reflection.invokeMethod(servletContext, "getRealPath", new Class[]{String.class}, "/");
            if (realPath instanceof String) {
                String separator = System.getProperty("file.separator");
                String rp = (String) realPath;
                if (rp.endsWith(separator)) {
                    rp = rp.substring(0, rp.length() - 1);
                }
                int index = rp.lastIndexOf(separator);
                return rp.substring(0, index);
            } else {
                return "";
            }
        } catch (Exception e) {
            e.printStackTrace();
            return "";
        }
    }
}
