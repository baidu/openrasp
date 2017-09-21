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

import java.io.ByteArrayOutputStream;
import java.util.Enumeration;
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
    protected static final int maxBodySize = 4096;
    protected String requestId;

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
     * 返回HTTP request body
     *
     * @return request body, can be null
     */
    public byte[] getBody() {
        return bodyOutputStream != null ? bodyOutputStream.toByteArray() : null;
    }

    /**
     * 返回input stream
     * @return input stream
     */
    public Object getInputStream() {
        return inputStream;
    }

    /**
     * 设置input stream
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
}
