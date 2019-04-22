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

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.tool.Reflection;
import org.apache.commons.lang3.StringUtils;

import java.io.ByteArrayOutputStream;
import java.io.CharArrayWriter;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

/**
 * Created by zhuming01 on 6/23/17.
 * All rights reserved
 * 为不同服务器的不同请求hook点做出的统一格式抽象类
 */
public abstract class AbstractRequest {
    protected static final Class[] EMPTY_CLASS = new Class[]{};
    protected static final Class[] STRING_CLASS = new Class[]{String.class};
    protected Object request;
    protected Object inputStream = null;
    protected ByteArrayOutputStream bodyOutputStream = null;
    protected CharArrayWriter bodyCharWriter = null;
    protected int maxBodySize = 4096;
    protected String requestId;
    protected boolean canGetParameter = false;
    protected HashMap<String, String[]> fileUploadCache = null;

    /**
     * constructor
     *
     * @see AbstractRequest#AbstractRequest(Object) 默认请求实体为null
     */
    public AbstractRequest() {
        this(null);
    }

    /**
     * constructor
     *
     * @param request 请求实体
     */
    public AbstractRequest(Object request) {
        this.request = request;
        this.requestId = UUID.randomUUID().toString().replace("-", "");
        this.maxBodySize = Config.getConfig().getBodyMaxBytes();
    }

    /**
     * constructor 测试时使用的构造函数
     */
    public AbstractRequest(int request) {
    }

    /**
     * 返回是否当前请求能够获取参数内容
     *
     * @return 是否能够获取参数内容
     */
    public boolean isCanGetParameter() {
        return canGetParameter;
    }

    /**
     * 设置是否能够获取参数
     *
     * @param canGetParameter 是否能够获取参数内容
     */
    public void setCanGetParameter(boolean canGetParameter) {
        this.canGetParameter = canGetParameter;
    }

    /**
     * 设置请求实体，该请求实体在不同的环境中可能是不同的类型
     *
     * @param request 请求实体
     */
    public void setRequest(Object request) {
        this.request = request;
    }

    /**
     * 获取请求实体
     *
     * @return 请求实体
     */
    public Object getRequest() {
        return this.request;
    }

    /**
     * 获取请求Id
     *
     * @return 请求Id
     */
    public String getRequestId() {
        return requestId;
    }

    /**
     * 获取本服务器地址
     *
     * @return 服务器地址
     */
    public abstract String getLocalAddr();

    /**
     * 获取请求方法
     *
     * @return 请求方法
     */
    public abstract String getMethod();

    /**
     * 获取请求协议
     *
     * @return 请求协议
     */
    public abstract String getProtocol();

    /**
     * 获取验证类型
     *
     * @return 验证类型
     */
    public abstract String getAuthType();

    /**
     * 获取请求路径
     *
     * @return 请求路径
     */
    public abstract String getContextPath();

    /**
     * 获取访问客户端的地址
     *
     * @return 客户端地址
     */
    public abstract String getRemoteAddr();

    /**
     * 获取请求的uri
     *
     * @return 请求uri
     */
    public abstract String getRequestURI();

    /**
     * 获取请求的url
     *
     * @return 请求的url
     */
    public abstract StringBuffer getRequestURL();

    public String getRequestURLString() {
        Object ret = getRequestURL();
        return ret != null ? ret.toString() : null;
    }

    /**
     * 获取服务器名称
     *
     * @return 服务器名称
     */
    public abstract String getServerName();

    /**
     * 根据请求的参数名称，获取请求参数的值
     *
     * @param key 请求参数名称
     * @return 请求参数的值
     */
    public abstract String getParameter(String key);

    /**
     * 获取所有请求参数名称
     *
     * @return 请求参数名称的枚举集合
     */
    public abstract Enumeration<String> getParameterNames();

    /**
     * 获取请求参数的map键值对集合
     * key为参数名称，value为参数值
     *
     * @return 请求参数的map集合
     */
    public abstract Map<String, String[]> getParameterMap();

    /**
     * 根据请求头的名称获取请求头的值
     *
     * @param key 请求头的名称
     * @return 请求头的值
     */
    public abstract String getHeader(String key);

    /**
     * 获取所有请求头的名称
     *
     * @return 请求头名称的枚举集合
     */
    public abstract Enumeration<String> getHeaderNames();

    public String[] getHeadersArray() {
        ArrayList<String> headers = new ArrayList<String>();
        Enumeration<String> headerNames = getHeaderNames();
        if (headerNames != null) {
            while (headerNames.hasMoreElements()) {
                String key = headerNames.nextElement();
                String value = getHeader(key);
                headers.add(key.toLowerCase());
                headers.add(value);
            }
        }
        return headers.toArray(new String[0]);
    }

    /**
     * 获取请求的url中的 Query String 参数部分
     *
     * @return 请求的 Query String
     */
    public abstract String getQueryString();

    /**
     * 获取服务器的上下文参数map集合
     * key为参数名字，value为参数的值
     *
     * @return 服务器上下文参数的map集合
     */
    public abstract Map<String, String> getServerContext();

    /**
     * 获取app部署根路径
     *
     * @return app部署根路径
     */
    public abstract String getAppBasePath();

    /**
     * 获取请求的contentType
     *
     * @return contentType
     */
    public abstract String getContentType();

    /**
     * 获取自定义的clientip
     *
     * @return 自定义的clientip
     */
    public abstract String getClinetIp();

    /**
     * 返回HTTP request body
     *
     * @return request body, can be null
     */
    public byte[] getBody() {
        return bodyOutputStream != null ? bodyOutputStream.toByteArray() : null;
    }

    /**
     * 返回HTTP request body stream
     *
     * @return request body, can be null
     */
    public ByteArrayOutputStream getBodyStream() {
        return bodyOutputStream;
    }

    /**
     * 返回input stream
     *
     * @return input stream
     */
    public Object getInputStream() {
        return inputStream;
    }

    /**
     * 设置input stream
     *
     * @param inputStream input stream
     */
    public void setInputStream(Object inputStream) {
        this.inputStream = inputStream;
    }

    /**
     * 添加HTTP request body
     *
     * @param b 要添加的字节
     */
    public void appendBody(int b) {
        if (bodyOutputStream == null) {
            bodyOutputStream = new ByteArrayOutputStream();
        }

        if (bodyOutputStream.size() < maxBodySize) {
            bodyOutputStream.write(b);
        }
    }

    /**
     * 添加HTTP request body
     *
     * @param bytes 要添加的字节数组
     */
    public void appendBody(byte[] bytes) {
        appendBody(bytes, 0, bytes.length);
    }

    /**
     * 添加HTTP request body
     *
     * @param bytes  字节数组
     * @param offset 要添加的起始偏移量
     * @param len    要添加的长度
     */
    public void appendBody(byte[] bytes, int offset, int len) {
        if (bodyOutputStream == null) {
            bodyOutputStream = new ByteArrayOutputStream();
        }

        len = Math.min(len, maxBodySize - bodyOutputStream.size());
        if (len > 0) {
            bodyOutputStream.write(bytes, offset, len);
        }
    }

    /**
     * 返回body的编码类型
     *
     * @return CharacterEncoding
     */
    public abstract String getCharacterEncoding();

    protected boolean setCharacterEncodingFromConfig() {
        try {
            String paramEncoding = Config.getConfig().getRequestParamEncoding();
            if (!StringUtils.isEmpty(paramEncoding)) {
                Reflection.invokeMethod(request, "setCharacterEncoding", STRING_CLASS, paramEncoding);
                return true;
            }
        } catch (Exception e) {
            String message = "set character encoding failed";
            int errorCode = ErrorType.RUNTIME_ERROR.getCode();
            HookHandler.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
        }
        return false;
    }

    public HashMap<String, String[]> getFileUploadCache() {
        return fileUploadCache;
    }

    public void setFileUploadCache(HashMap<String, String[]> cache) {
        fileUploadCache = cache;
    }
}
