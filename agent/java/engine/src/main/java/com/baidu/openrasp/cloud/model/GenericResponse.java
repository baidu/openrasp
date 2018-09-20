package com.baidu.openrasp.cloud.model;

import java.util.Map;

/**
 * @description: 通用的响应model
 * @author: anyang
 * @create: 2018/09/17 17:56
 */
public class GenericResponse {
    private int responseCode;
    private int status;
    private String description;
    private Map<String,Object> data;

    public int getResponseCode() {
        return responseCode;
    }

    public void setResponseCode(int responseCode) {
        this.responseCode = responseCode;
    }

    public int getStatus() {
        return status;
    }

    public void setStatus(int status) {
        this.status = status;
    }

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    public Map<String, Object> getData() {
        return data;
    }

    public void setData(Map<String, Object> data) {
        this.data = data;
    }
}
