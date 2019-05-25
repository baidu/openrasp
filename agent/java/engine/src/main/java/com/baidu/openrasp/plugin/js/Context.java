package com.baidu.openrasp.plugin.js;

import java.util.*;
import com.baidu.openrasp.request.AbstractRequest;
import com.jsoniter.output.JsonStream;
import com.baidu.openrasp.v8.ByteArrayOutputStream;

public class Context implements com.baidu.openrasp.v8.Context {
    public AbstractRequest request = null;

    public Context(AbstractRequest request) {
        this.request = request;
    }

    public String getPath() {
        try {
            return request.getRequestURI();
        } catch (Exception e) {
            return null;
        }
    }

    public String getMethod() {
        try {
            return request.getMethod().toLowerCase();
        } catch (Exception e) {
            return null;
        }
    }

    public String getUrl() {
        try {
            return request.getRequestURL().toString();
        } catch (Exception e) {
            return null;
        }
    }

    public String getQuerystring() {
        try {
            return request.getQueryString();
        } catch (Exception e) {
            return null;
        }
    }

    public String getAppBasePath() {
        try {
            return request.getAppBasePath();
        } catch (Exception e) {
            return null;
        }
    }

    public String getProtocol() {
        try {
            return request.getProtocol().toLowerCase();
        } catch (Exception e) {
            return null;
        }
    }

    public String getRemoteAddr() {
        try {
            return request.getRemoteAddr();
        } catch (Exception e) {
            return null;
        }
    }

    public byte[] getBody(int[] size) {
        try {
            java.io.ByteArrayOutputStream body = request.getBodyStream();
            size[0] = body.size();
            return body.toByteArray();
        } catch (Exception e) {
            return null;
        }
    }

    public byte[] getJson(int[] size) {
        try {
            String contentType = request.getContentType();
            if (contentType != null && contentType.contains("application/json")) {
                return getBody(size);
            }
            return null;
        } catch (Exception e) {
            return null;
        }
    }

    public byte[] getHeader(int[] size) {
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
            size[0] = out.size();
            return out.getByteArray();
        } catch (Exception e) {
            return null;
        }
    }

    public byte[] getParameter(int[] size) {
        try {
            Map<String, String[]> parameters = request.getParameterMap();
            if (parameters == null || parameters.isEmpty()) {
                return null;
            }
            ByteArrayOutputStream out = new ByteArrayOutputStream();
            JsonStream.serialize(parameters, out);
            size[0] = out.size();
            return out.getByteArray();
        } catch (Exception e) {
            return null;
        }
    }

    public byte[] getServer(int[] size) {
        try {
            Map<String, String> server = request.getServerContext();
            ByteArrayOutputStream out = new ByteArrayOutputStream();
            JsonStream.serialize(server, out);
            size[0] = out.size();
            return out.getByteArray();
        } catch (Exception e) {
            return null;
        }
    }
}