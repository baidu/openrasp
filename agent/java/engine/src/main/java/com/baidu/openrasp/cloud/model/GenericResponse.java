package com.baidu.openrasp.cloud.model;

import java.util.Map;

/**
 * @description: 通用的响应model
 * @author: anyang
 * @create: 2018/09/17 17:56
 */
public class GenericResponse {
    private Integer responseCode;
    private Integer status;
    private String description;
    private Map<String,Object> data;

    public Integer getResponseCode() {
        return responseCode;
    }

    public void setResponseCode(Integer responseCode) {
        this.responseCode = responseCode;
    }

    public Integer getStatus() {
        return status;
    }

    public void setStatus(Integer status) {
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
