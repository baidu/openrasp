package com.baidu.openrasp.plugin.v8;

import java.util.*;
import com.baidu.openrasp.request.AbstractRequest;
import com.jsoniter.output.JsonStream;

public class Context {
    public AbstractRequest request = null;

    public Context(AbstractRequest request) {
        this.request = request;
    }

    public String getPath() {
        String path = request.getRequestURI();
        return path == null ? null : path;
    }

    public String getMethod() {
        String method = request.getMethod();
        return method == null ? null : method.toLowerCase();
    }

    public String getUrl() {
        StringBuffer requestURL = request.getRequestURL();
        return requestURL == null ? null : requestURL.toString();
    }

    public String getQuerystring() {
        String query = request.getQueryString();
        return query == null ? null : query;
    }

    public String getAppBasePath() {
        String appBasePath = request.getAppBasePath();
        return appBasePath == null ? null : appBasePath;
    }

    public String getProtocol() {
        String proto = request.getProtocol();
        return proto == null ? null : proto.toLowerCase();
    }

    public String getRemoteAddr() {
        String remoteAddr = request.getRemoteAddr();
        return remoteAddr == null ? null : remoteAddr;
    }

    public byte[] getBody(int[] size) {
        java.io.ByteArrayOutputStream body = request.getBodyStream();
        if (body == null) {
            return null;
        } else {
            size[0] = body.size();
            return body.toByteArray();
        }
    }

    public String getJson() {
        String contentType = request.getContentType();
        if (contentType != null && contentType.contains("application/json")) {
            byte[] body = request.getBody();
            if (body != null) {
                return new String(body);
            }
        }
        return null;
    }

    public byte[] getHeader(int[] size) {
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
        V8ByteArrayOutputStream out = new V8ByteArrayOutputStream();
        JsonStream.serialize(headers, out);
        size[0] = out.size();
        return out.getByteArray();
    }

    public byte[] getParameter(int[] size) {
        Map<String, String[]> parameters = request.getParameterMap();
        if (parameters == null || parameters.isEmpty()) {
            return null;
        }
        V8ByteArrayOutputStream out = new V8ByteArrayOutputStream();
        JsonStream.serialize(parameters, out);
        size[0] = out.size();
        return out.getByteArray();
    }

    public byte[] getServer(int[] size) {
        Map<String, String> server = request.getServerContext();
        V8ByteArrayOutputStream out = new V8ByteArrayOutputStream();
        JsonStream.serialize(server, out);
        size[0] = out.size();
        return out.getByteArray();
    }
}