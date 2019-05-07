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

    public String getJson() {
        try {
            String contentType = request.getContentType();
            if (contentType != null && contentType.contains("application/json")) {
                byte[] body = request.getBody();
                if (body != null) {
                    return new String(body);
                }
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
            V8ByteArrayOutputStream out = new V8ByteArrayOutputStream();
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
            V8ByteArrayOutputStream out = new V8ByteArrayOutputStream();
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
            V8ByteArrayOutputStream out = new V8ByteArrayOutputStream();
            JsonStream.serialize(server, out);
            size[0] = out.size();
            return out.getByteArray();
        } catch (Exception e) {
            return null;
        }
    }
}