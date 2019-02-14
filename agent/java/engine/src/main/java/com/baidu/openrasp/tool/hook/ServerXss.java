package com.baidu.openrasp.tool.hook;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.plugin.js.engine.JSContext;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * 　　* @Description: XSS检测hook点工具类
 * 　　* @author anyang
 * 　　* @date 2018/8/7 16:55
 */
public class ServerXss {
    private static final int DEFAULT_MIN_LENGTH = 15;
    private static final String CONFIG_KEY_XSS_USER_INPUT = "xss_userinput";
    private static final String XSS_PARAMETER_LENGTH = "min_length";
    private static final String XSS_REGEX = "filter_regex";


    /**
     * 生成XSS检测js插件所需参数
     *
     * @param content 待检测HTML
     */
    public static HashMap<String, Object> generateXssParameters(String content) {
        JsonObject config = Config.getConfig().getAlgorithmConfig();
        int parameterLength = getIntElement(config, CONFIG_KEY_XSS_USER_INPUT, XSS_PARAMETER_LENGTH);
        String regex = getStringElement(config, CONFIG_KEY_XSS_USER_INPUT, XSS_REGEX);
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

    private static String getStringElement(JsonObject config, String key, String subKey) {
        if (config != null) {
            try {
                JsonElement jsonElement = config.get(key);
                if (jsonElement != null) {
                    JsonElement value = jsonElement.getAsJsonObject().get(subKey);
                    return value.getAsString();
                }
            } catch (Exception e) {
                String message = "Parse json failed";
                int errorCode = ErrorType.PLUGIN_ERROR.getCode();
                JSContext.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
            }
        }
        return null;
    }

    private static int getIntElement(JsonObject config, String key, String subKey) {
        if (config != null) {
            try {
                JsonElement jsonElement = config.get(key);
                if (jsonElement != null) {
                    JsonElement value = jsonElement.getAsJsonObject().get(subKey);
                    return value != null ? value.getAsInt() : DEFAULT_MIN_LENGTH;
                }
            } catch (Exception e) {
                String message = "Parse json failed";
                int errorCode = ErrorType.PLUGIN_ERROR.getCode();
                JSContext.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
            }
        }
        return DEFAULT_MIN_LENGTH;
    }
}
