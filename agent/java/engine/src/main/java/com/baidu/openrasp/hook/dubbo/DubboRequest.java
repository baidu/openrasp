package com.baidu.openrasp.hook.dubbo;


import java.util.HashMap;
import java.util.Map;

/**
 * @author anyang
 * @Description: dubbo的request请求类
 * @date 2018/8/1315:13
 */
public class DubboRequest extends AbstractDubboRequest {

    private static final Map<String, String[]> EMPTY_PARAM = new HashMap<String, String[]>();


    public DubboRequest(Object request) {
        super(request);
    }

    @Override
    public Map<String, String[]> getParameterMap() {

        return request != null ? (Map<String, String[]>) request : EMPTY_PARAM;
    }
}
