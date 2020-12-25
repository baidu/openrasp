/*
 * Copyright 2017-2021 Baidu Inc.
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

package com.baidu.openrasp.plugin.checker.policy;

import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.messaging.ErrorType;
import com.baidu.openrasp.messaging.LogTool;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.info.EventInfo;
import com.baidu.openrasp.plugin.info.SecurityPolicyInfo;
import com.baidu.openrasp.tool.LRUCache;
import com.baidu.openrasp.tool.OSUtil;

import java.security.NoSuchAlgorithmException;
import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * @description: 敏感日志检测checker
 * @author: anyang
 * @create: 2019/06/12 18:54
 */
public class LogChecker extends PolicyChecker {

    private static LRUCache<String, Object> logCache = new LRUCache<String, Object>(100);

    public LogChecker(boolean canBlock) {
        super(canBlock);
    }

    @Override
    public List<EventInfo> checkParam(CheckParameter checkParameter) {
        LinkedList<EventInfo> result = new LinkedList<EventInfo>();
        String logMessage = (String) checkParameter.getParam("message");
        if (logMessage != null) {
            try {
                Map<String, String> logRegexes = Config.getConfig().getLogSensitiveRegex();
                if (logRegexes != null) {
                    for (Map.Entry<String, String> element : logRegexes.entrySet()) {
                        String type = element.getKey();
                        String regex = element.getValue();
                        String md5 = null;
                        try {
                            md5 = OSUtil.getDigestMd5(stringify(new Throwable().getStackTrace()) + regex);
                        } catch (NoSuchAlgorithmException e) {
                            // ignore
                        }
                        if (md5 != null && !logCache.isContainsKey(md5)) {
                            Pattern pattern = Pattern.compile(regex);
                            Matcher matcher = pattern.matcher(logMessage);
                            while (matcher.find()) {
                                logCache.put(md5, null);
                                String sensitiveData = matcher.group(0);
                                sensitiveData = maskSensitiveData(sensitiveData);
                                String message = type + " detected in the log message, value is: " + sensitiveData;
                                HashMap<String, Object> params = new HashMap<String, Object>(7);
                                params.put("type", type);
                                params.put("message", type);
                                result.add(new SecurityPolicyInfo(SecurityPolicyInfo.Type.SENSITIVE_LOG,
                                        message, false, checkParameter.getParams()));
                            }
                        }
                    }
                }
            } catch (Exception e) {
                LogTool.warn(ErrorType.PLUGIN_ERROR, "log message detected failed: " + e.getMessage(), e);
            }

        }
        return result;
    }

    private String maskSensitiveData(String data) {
        StringBuilder maskData = new StringBuilder(data);
        if (data.length() >= 3) {
            int maskLength = data.length() / 3;
            char[] mask = new char[maskLength];
            Arrays.fill(mask, '*');
            maskData.replace(maskLength, maskLength * 2, new String(mask));
        }
        return maskData.toString();
    }

    private String stringify(StackTraceElement[] trace) {
        StringBuilder sb = new StringBuilder();
        for (StackTraceElement element : trace) {
            sb.append(element.toString());
        }
        return sb.toString();
    }
}
