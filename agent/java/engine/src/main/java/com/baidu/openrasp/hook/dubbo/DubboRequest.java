package com.baidu.openrasp.hook.dubbo;


import java.util.HashMap;
import java.util.Map;

/**
 * Created by anyang on 2018/6/22.
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
