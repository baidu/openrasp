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
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Created by zhuming01 on 6/15/17.
 * All rights reserved
 * javax.servlet.http.HttpServletRequest类请求的统一格式接口
 */
public final class HttpServletRequest extends AbstractRequest {
    private static final Map<String, String[]> EMPTY_PARAM = new HashMap<String, String[]>();
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
        if (canGetParameter) {
            return Reflection.invokeStringMethod(request, "getParameter", STRING_CLASS, key);
        }
        return null;
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getParameterNames()
     */
    @Override
    public Enumeration<String> getParameterNames() {
        if (canGetParameter) {
            Object ret = Reflection.invokeMethod(request, "getParameterNames", EMPTY_CLASS);
            return ret != null ? (Enumeration) ret : null;
        } else {
            return null;
        }
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractRequest#getParameterMap()
     */
    @Override
    public Map<String, String[]> getParameterMap() {
        if (canGetParameter) {
            Object ret = Reflection.invokeMethod(request, "getParameterMap", EMPTY_CLASS);
            return ret != null ? (Map<String, String[]>) ret : EMPTY_PARAM;
        } else {
            return EMPTY_PARAM;
        }
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
        Object servletContext = getServletContextObject();
        String serverInfo = Reflection.invokeStringMethod(servletContext, "getServerInfo", EMPTY_CLASS);

        Map<String, String> ret = new HashMap<String, String>();
        // TODO more reliable
        ret.put("server", extractType(serverInfo));
        ret.put("version", extractNumber(serverInfo));
        ret.put("os", getOs(System.getProperty("os.name")));
        ret.put("language", "java");
        return ret;
    }

    public static String getOs(String os) {
        if (os == null) {
            return null;
        }
        os = os.toLowerCase();
        if (os.contains("linux")) return "Linux";
        if (os.contains("windows")) return "Windows";
        if (os.contains("mac")) return "Mac";
        if (os.contains("sunos")) return "SunOS";
        if (os.contains("freebsd")) return "FreeBSD";
        return os;
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
        Object session = getSessionObject();
        Reflection.invokeMethod(session, "setAttribute", new Class[]{String.class, Object.class}, key, value);
    }

    /**
     * 获取指定键对应session
     *
     * @param key 键
     */
    public Object getSessionAttribute(String key) {
        Object session = getSessionObject();
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
            Object servletContext = getServletContextObject();
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

    //--------------------------------私有方法-------------------------------------

    /**
     * 反射获取session object
     *
     * @return
     */
    private Object getSessionObject() {
        return Reflection.invokeMethod(request, "getSession", new Class[]{boolean.class}, true);
    }

    /**
     * 反射获取servletContext object
     *
     * @return
     */
    private Object getServletContextObject() {
        return Reflection.invokeMethod(getSessionObject(), "getServletContext", EMPTY_CLASS);
    }
}
