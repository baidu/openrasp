package com.baidu.openrasp.tool.hook;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.config.Config;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
　　* @Description: XSS检测hook点工具类
　　* @author anyang
　　* @date 2018/8/7 16:55
　　*/
public class ServerXss {

    /**
     * 生成XSS检测js插件所需参数
     *
     * @param content 待检测HTML
     */
    public static HashMap<String, Object> generateXssParameters(String content) {
        int parameterLength = Integer.valueOf(Config.getConfig().getXssParameterLength());
        String regex = Config.getConfig().getXssRegex();
        HashMap<String, Object> params = null;
        Map<String, String[]> parameterMap = HookHandler.requestCache.get().getParameterMap();
        int exceedLengthCount = 0;
        List<String> paramList = new ArrayList<String>();
        for (Map.Entry<String, String[]> entry : parameterMap.entrySet()) {

            for (String value : entry.getValue()) {
                Pattern pattern = Pattern.compile(regex);
                Matcher matcher = pattern.matcher(value);
                boolean isMatch = matcher.find();
                if (value.length() >= parameterLength && isMatch) {
                    exceedLengthCount++;
                    paramList.add(value);
                }
            }
        }
        params = new HashMap<String, Object>(3);
        params.put("exceed_count", exceedLengthCount);
        params.put("html_body", content);
        params.put("param_list", paramList);
        return params;
    }
}
