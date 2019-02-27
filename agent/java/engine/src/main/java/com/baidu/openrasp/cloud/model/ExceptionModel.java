package com.baidu.openrasp.cloud.model;

/**
 * @description: 异常对象
 * @author: anyang
 * @create: 2019/01/18 10:55
 */
public class ExceptionModel {

    private String message;
    private int errorCode;

    public String getMessage() {
        return message;
    }

    public void setMessage(String message) {
        this.message = message;
    }

    public int getErrorCode() {
        return errorCode;
    }

    public void setErrorCode(int errorCode) {
        this.errorCode = errorCode;
    }

    @Override
    public String toString() {
        return "[E" + errorCode + "] " + message + ": ";
    }
}
