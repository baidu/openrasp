package com.baidu.openrasp.hook.dubbo;

import java.util.Map;

/**
 * @author anyang
 * @Description: dubbo的request请求抽象类
 * @date 2018/8/1315:13
 */
public abstract class AbstractDubboRequest {

    protected static final Class[] EMPTY_CLASS = new Class[]{};

    public Object request;

    public AbstractDubboRequest(Object request) {

        this.request=request;
    }

    public Object getRequest() {
        return request;
    }

    public void setRequest(Object request) {
        this.request = request;
    }


    /**
     * 获取dubbo请求参数的map键值对集合
     * key为参数名称，value为参数值
     *
     * @return 请求参数的map集合
     */
    public abstract Map<String, String[]> getParameterMap();
}
