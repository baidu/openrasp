package com.baidu.openrasp.request;

/**
 * Created by tyy on 19-10-29.
 * 请求携带的文件信息
 */
public class RequestFileItem {

    private final String name;
    private final String fileName;

    public RequestFileItem(String name, String fileName) {
        this.name = name;
        this.fileName = fileName;
    }

    public String getName() {
        return name;
    }

    public String getFileName() {
        return fileName;
    }

}
