/*
 * Copyright 2017-2019 Baidu Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.baidu.openrasp.plugin.checker.local;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.info.AttackInfo;
import com.baidu.openrasp.plugin.info.EventInfo;
import com.baidu.openrasp.tool.LRUCache;
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import org.apache.commons.lang3.StringUtils;

import java.util.LinkedList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * @description: 敏感日志检测checker
 * @author: anyang
 * @create: 2019/06/12 18:54
 */
public class LogChecker extends ConfigurableChecker {
    private static final String LOG_REGEX = "log_regex";
    private static final String ACTION = "log";
    private static JsonArray defaultRegex;

    static {
        defaultRegex = new JsonArray();
        JsonObject object = new JsonObject();
        object.addProperty("name", "身份证");
        object.addProperty("value", "(\\d{14}[0-9a-zA-Z])|(\\d{17}[0-9a-zA-Z])");
        defaultRegex.add(object);
    }

    private static LRUCache<String, Object> logCache = new LRUCache<String, Object>(100);

    @Override
    public List<EventInfo> checkParam(CheckParameter checkParameter) {
        JsonObject config = Config.getConfig().getAlgorithmConfig();
        LinkedList<EventInfo> result = new LinkedList<EventInfo>();
        String logMessage = (String) checkParameter.getParam("message");
        if (logMessage != null) {
            try {
                JsonArray regexs = getJsonObjectAsArray(config, LOG_REGEX, "regex");
                if (regexs == null) {
                    regexs = defaultRegex;
                }
                if (regexs != null) {
                    for (JsonElement element : regexs) {
                        JsonObject jsonObject = element.getAsJsonObject();
                        String type = jsonObject.get("name").getAsString();
                        String regex = jsonObject.get("value").getAsString();
                        Pattern pattern = Pattern.compile(regex);
                        Matcher matcher = pattern.matcher(logMessage);
                        boolean isMatch = matcher.find();
                        if (isMatch) {
                            if (type.equals("身份证")) {
                                String stackTrace = stringify(new Throwable().getStackTrace());
                                String id = matcher.group(0);
                                if (!StringUtils.isEmpty(id) && !logCache.isContainsKey(stackTrace + regex + id)) {
                                    logCache.put(stackTrace + regex + id, null);
                                    id = formatIdCard(id);
                                    String message = type + " detected in the log message, value is: " + id;
                                    result.add(AttackInfo.createLocalAttackInfo(checkParameter, ACTION, message, LOG_REGEX));
                                    return result;
                                }
                            }
                        }
                    }
                }
            } catch (Exception e) {
                String msg = "log message detected failed";
                int code = ErrorType.PLUGIN_ERROR.getCode();
                HookHandler.LOGGER.warn(CloudUtils.getExceptionObject(msg, code), e);
            }

        }
        return result;
    }

    private String formatIdCard(String id) {
        StringBuilder sb = new StringBuilder(id);
        sb.replace(6, 10, "****");
        return sb.toString();
    }

    private String stringify(StackTraceElement[] trace) {
        StringBuilder sb = new StringBuilder();
        for (StackTraceElement element : trace) {
            sb.append(element.toString());
        }
        return sb.toString();
    }
}
