package com.baidu.openrasp.hook.dubbo;

import com.baidu.openrasp.tool.Reflection;

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

        Object ret = Reflection.invokeMethod(request, "getParameterMap", EMPTY_CLASS);

        return ret != null ? (Map<String, String[]>) ret : EMPTY_PARAM;
    }
}
