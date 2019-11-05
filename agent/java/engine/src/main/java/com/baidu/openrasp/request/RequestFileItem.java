package com.baidu.openrasp.request;

/**
 * Created by tyy on 19-10-29.
 * 请求携带的文件信息
 */
public class RequestFileItem {

    private final String name;
    private final String filename;

    public RequestFileItem(String name, String fileName) {
        this.name = name;
        this.filename = fileName;
    }

    public String getName() {
        return name;
    }

    public String getFilename() {
        return filename;
    }

}
