/*
 * Copyright 2017-2021 Baidu Inc.
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

package com.baidu.openrasp.plugin.js;

import com.baidu.openrasp.cloud.model.CloudCacheModel;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.request.AbstractRequest;
import com.baidu.openrasp.tool.OSUtil;
import com.baidu.openrasp.tool.model.ApplicationModel;
import com.baidu.openrasp.tool.model.NicModel;
import com.baidu.openrasp.v8.ByteArrayOutputStream;
import com.jsoniter.output.JsonStream;

import java.io.UnsupportedEncodingException;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class Context extends com.baidu.openrasp.v8.Context {

    public AbstractRequest request = null;

    public static void setKeys() {
        setStringKeys(new String[]{"path", "method", "url", "querystring", "protocol", "remoteAddr", "appBasePath",
                "requestId", "appId", "raspId", "hostname", "source", "target", "clientIp"});
        setObjectKeys(new String[]{"json", "server", "parameter", "header", "nic"});
        setBufferKeys(new String[]{"body"});
    }

    public Context(AbstractRequest request) {
        this.request = request;
    }

    public String getString(String key) {
        if (key.equals("path"))
            return getPath();
        if (key.equals("method"))
            return getMethod();
        if (key.equals("url"))
            return getUrl();
        if (key.equals("querystring"))
            return getQuerystring();
        if (key.equals("appBasePath"))
            return getAppBasePath();
        if (key.equals("protocol"))
            return getProtocol();
        if (key.equals("remoteAddr"))
            return getRemoteAddr();
        if (key.equals("requestId"))
            return getRequestId();
        if (key.equals("appId"))
            return getAppId();
        if (key.equals("raspId"))
            return getRaspId();
        if (key.equals("hostname"))
            return getHostname();
        if (key.equals("source"))
            return getSource();
        if (key.equals("target"))
            return getTarget();
        if (key.equals("clientIp"))
            return getClientIp();
        return null;
    }

    public byte[] getObject(String key) {
        if (key.equals("json"))
            return getJson();
        if (key.equals("header"))
            return getHeader();
        if (key.equals("parameter"))
            return getParameter();
        if (key.equals("server"))
            return getServer();
        if (key.equals("nic"))
            return getNic();
        return null;
    }

    public byte[] getBuffer(String key) {
        if (key.equals("body"))
            return getBody();
        return null;
    }

    public String getPath() {
        try {
            return request.getRequestURI().toString();
        } catch (Exception e) {
            return "";
        }
    }

    public String getMethod() {
        try {
            return request.getMethod().toLowerCase().toString();
        } catch (Exception e) {
            return "";
        }
    }

    public String getUrl() {
        try {
            return request.getRequestURL().toString().toString();
        } catch (Exception e) {
            return "";
        }
    }

    public String getQuerystring() {
        try {
            return request.getQueryString().toString();
        } catch (Exception e) {
            return "";
        }
    }

    public String getAppBasePath() {
        try {
            return request.getAppBasePath().toString();
        } catch (Exception e) {
            return "";
        }
    }

    public String getProtocol() {
        try {
            return request.getProtocol().toLowerCase().toString();
        } catch (Exception e) {
            return "";
        }
    }

    public String getRemoteAddr() {
        try {
            return request.getRemoteAddr().toString();
        } catch (Exception e) {
            return "";
        }
    }

    public String getRequestId() {
        try {
            return request.getRequestId().toString();
        } catch (Exception e) {
            return "";
        }
    }

    // TODO: update openrasp-v8, accept string body
    public byte[] getBody() {
        try {
            return escape(request.getStringBody());
        } catch (Exception e) {
            return null;
        }
    }

    public byte[] getJson() {
        try {
            String contentType = request.getContentType();
            if (contentType != null && contentType.contains("application/json")) {
                return getBody();
            }
            return null;
        } catch (Exception e) {
            return null;
        }
    }

    public byte[] escape(String src) throws UnsupportedEncodingException {
        char j;
        StringBuilder tmp = new StringBuilder();
        for (int i = 0; i < src.length(); i++) {
            j = src.charAt(i);
            if (j < 256)
                tmp.append(j);
            else {
                tmp.append("\\u");
                tmp.append(Integer.toString(j, 16));
            }
        }
        return tmp.toString().getBytes("UTF-8");
    }

    public byte[] getHeader() {
        try {
            Enumeration<String> headerNames = request.getHeaderNames();
            if (headerNames == null || !headerNames.hasMoreElements()) {
                return null;
            }
            HashMap<String, String> headers = new HashMap<String, String>();
            while (headerNames.hasMoreElements()) {
                String key = headerNames.nextElement();
                String value = request.getHeader(key);
                headers.put(key.toLowerCase(), value);
            }
            ByteArrayOutputStream out = new ByteArrayOutputStream();
            JsonStream.serialize(headers, out);
            out.write(0);
            return out.getByteArray();
        } catch (Exception e) {
            return "{}".getBytes();
        }
    }

    public byte[] getParameter() {
        try {
            Map<String, String[]> parameters = request.getParameterMap();
            if (parameters == null || parameters.isEmpty()) {
                return null;
            }
            ByteArrayOutputStream out = new ByteArrayOutputStream();
            JsonStream.serialize(parameters, out);
            out.write(0);
            return out.getByteArray();
        } catch (Exception e) {
            return null;
        }
    }

    public byte[] getServer() {
        try {
            Map<String, String> server = ApplicationModel.getApplicationInfo();
            ByteArrayOutputStream out = new ByteArrayOutputStream();
            JsonStream.serialize(server, out);
            out.write(0);
            return out.getByteArray();
        } catch (Exception e) {
            return "{}".getBytes();
        }
    }

    public String getAppId() {
        try {
            return Config.getConfig().getCloudAppId();
        } catch (Exception e) {
            return "";
        }
    }

    public String getRaspId() {
        try {
            return CloudCacheModel.getInstance().getRaspId() != null ? CloudCacheModel.getInstance().getRaspId() : "";
        } catch (Exception e) {
            return "";
        }
    }

    public String getHostname() {
        try {
            return OSUtil.getHostName();
        } catch (Exception e) {
            return "";
        }
    }

    public byte[] getNic() {
        try {
            List<NicModel> nic = OSUtil.getIpAddress();
            ByteArrayOutputStream out = new ByteArrayOutputStream();
            JsonStream.serialize(nic, out);
            out.write(0);
            return out.getByteArray();
        } catch (Exception e) {
            return "{}".getBytes();
        }
    }

    public String getSource() {
        try {
            return request.getRemoteAddr();
        } catch (Exception e) {
            return "";
        }
    }

    public String getTarget() {
        try {
            return request.getLocalAddr();
        } catch (Exception e) {
            return "";
        }
    }

    public String getClientIp() {
        try {
            return request.getClientIp();
        } catch (Exception e) {
            return "";
        }
    }
}