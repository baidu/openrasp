/*
 * Copyright 2017-2018 Baidu Inc.
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

import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.info.AttackInfo;
import com.baidu.openrasp.plugin.info.EventInfo;
import com.google.gson.JsonObject;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

public class XssChecker extends ConfigurableChecker {
    private static final String CONFIG_KEY_XSS_USER_INPUT = "xss_userinput";
    private static final String EXCEED_LENGTH_COUNT = "max_detection_num";
    private static final String XSS_PARAMETER_LENGTH = "min_length";
    private static final int DEFAULT_MIN_LENGTH = 15;
    private static final int DEFAULT_MAX_DETECTION_NUM = 10;

    @Override
    public List<EventInfo> checkParam(CheckParameter checkParameter) {
        JsonObject config = Config.getConfig().getAlgorithmConfig();
        String action = getActionElement(config, CONFIG_KEY_XSS_USER_INPUT);
        String message = null;
        LinkedList<EventInfo> result = new LinkedList<EventInfo>();
        Integer exceedCount = (Integer) checkParameter.getParam("exceed_count");
        @SuppressWarnings("unchecked")
        List<String> paramList = (ArrayList<String>) checkParameter.getParam("param_list");
        String content = String.valueOf(checkParameter.getParam("html_body"));
        if (!EventInfo.CHECK_ACTION_IGNORE.equals(action)) {
            if (content != null && !paramList.isEmpty()) {
                for (String param : paramList) {
                    if (content.contains(param)) {
                        message = "请求参数" + param + "存在XSS攻击风险";
                    }
                }
            }
            if (message != null) {
                result.add(AttackInfo.createLocalAttackInfo(checkParameter, EventInfo.CHECK_ACTION_BLOCK, message, CONFIG_KEY_XSS_USER_INPUT));
            } else {
                if (exceedCount != null) {
                    int exceedLengthCount = getIntElement(config, CONFIG_KEY_XSS_USER_INPUT, EXCEED_LENGTH_COUNT);
                    if (exceedLengthCount < 0) {
                        exceedLengthCount = DEFAULT_MAX_DETECTION_NUM;
                    }
                    int xssParameterLength = getIntElement(config, CONFIG_KEY_XSS_USER_INPUT, XSS_PARAMETER_LENGTH);
                    if (xssParameterLength < 0) {
                        xssParameterLength = DEFAULT_MIN_LENGTH;
                    }
                    if (exceedCount >= exceedLengthCount) {
                        message = "所有的请求参数中长度大于等于" + xssParameterLength + "并且匹配XSS正则的数量超过了" + exceedLengthCount;
                    }
                }
                if (message != null) {
                    result.add(AttackInfo.createLocalAttackInfo(checkParameter, EventInfo.CHECK_ACTION_BLOCK, message, CONFIG_KEY_XSS_USER_INPUT));
                }
            }
        }
        return result;
    }
}
