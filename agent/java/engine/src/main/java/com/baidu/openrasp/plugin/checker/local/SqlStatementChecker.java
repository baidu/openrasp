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

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.TokenGenerator;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.plugin.antlrlistener.TokenizeErrorListener;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.checker.js.JsChecker;
import com.baidu.openrasp.plugin.info.AttackInfo;
import com.baidu.openrasp.plugin.info.EventInfo;
import com.baidu.openrasp.plugin.js.engine.JSContext;
import com.google.gson.JsonObject;
import org.apache.commons.lang3.StringUtils;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

/**
 * Created by tyy on 17-12-20.
 *
 * 检测 sql 语句的 java 版本
 */
public class SqlStatementChecker extends ConfigurableChecker {

    private static final String CONFIG_KEY_SQLI_USER_INPUT = "sqli_userinput";
    private static final String CONFIG_KEY_DB_MANAGER = "sqli_dbmanager";
    private static final String CONFIG_KEY_SQLI_POLICY = "sqli_policy";
    private static final String CONFIG_KEY_STACKED_QUERY = "stacked_query";
    private static final String CONFIG_KEY_NO_HEX = "no_hex";
    private static final String CONFIG_KEY_CONSTANT_COMPARE = "constant_compare";
    private static final String CONFIG_KEY_VERSION_COMMENT = "version_comment";
    private static final String CONFIG_KEY_FUNCTION_BLACKLIST = "function_blacklist";
    private static final String CONFIG_KEY_UNION_NULL = "union_null";

    private static TokenizeErrorListener tokenizeErrorListener = new TokenizeErrorListener();

    @Override
    public List<EventInfo> checkParam(CheckParameter checkParameter) {
        List<EventInfo> result = new LinkedList<EventInfo>();
        String query = (String) checkParameter.getParam("query");

        String message = null;
        String[] tokens = TokenGenerator.tokenize(query, tokenizeErrorListener);
        Map<String, String[]> parameterMap = HookHandler.requestCache.get().getParameterMap();
        try {
            JsonObject config = Config.getConfig().getAlgorithmConfig();
            // 算法1: 匹配用户输入
            // 1. 简单识别逻辑是否发生改变
            // 2. 识别数据库管理器
            String action = getActionElement(config, CONFIG_KEY_SQLI_USER_INPUT);
            if (!EventInfo.CHECK_ACTION_IGNORE.equals(action) && action != null && parameterMap != null) {
                for (Map.Entry<String, String[]> entry : parameterMap.entrySet()) {
                    String[] v = entry.getValue();
                    String value = v[0];
                    if (value.length() <= 15) {
                        continue;
                    }
                    if (value.length() == query.length() && value.equals(query)) {
                        String managerAction = getActionElement(config, CONFIG_KEY_DB_MANAGER);
                        if (!EventInfo.CHECK_ACTION_IGNORE.equals(managerAction) && managerAction != null) {
                            message = "算法2: WebShell - 数据库管理器 - 攻击参数: " + entry.getKey();
                            action = managerAction;
                            break;
                        } else {
                            continue;
                        }
                    }
                    if (!query.contains(value)) {
                        continue;
                    }
                    String[] tokens2 = TokenGenerator.tokenize(query.replace(value, ""), tokenizeErrorListener);
                    if (tokens != null) {
                        if (tokens.length - tokens2.length > 2) {
                            message = "算法1: 数据库查询逻辑发生改变 - 攻击参数: " + entry.getKey();
                            break;
                        }
                    }
                }
            }
            if (message != null) {
                result.add(AttackInfo.createLocalAttackInfo(checkParameter, action,
                        message, 90));
            } else {
                // 算法2: SQL语句策略检查（模拟SQL防火墙功能）
                action = getActionElement(config, CONFIG_KEY_SQLI_POLICY);
                if (!EventInfo.CHECK_ACTION_IGNORE.equals(action)) {
                    int i = -1;
                    if (tokens != null) {
                        HashMap<String, Boolean> modules = getJsonObjectAsMap(config, CONFIG_KEY_SQLI_POLICY, "feature");
                        for (String token : tokens) {
                            i++;
                            if (!StringUtils.isEmpty(token)) {
                                String lt = token.toLowerCase();
                                if (lt.equals("select")
                                        && modules.containsKey(CONFIG_KEY_UNION_NULL)
                                        && modules.get(CONFIG_KEY_UNION_NULL)) {
                                    int nullCount = 0;
                                    // 寻找连续的逗号、NULL或者数字
                                    for (int j = i + 1; j < tokens.length && j < i + 6; j++) {
                                        if (tokens[j].equals(",") || tokens[j].equals("null") || StringUtils.isNumeric(tokens[j])) {
                                            nullCount++;
                                        } else {
                                            break;
                                        }
                                    }

                                    // NULL,NULL,NULL == 5个token
                                    // 1,2,3          == 5个token
                                    if (nullCount >= 5) {
                                        message = "UNION-NULL 方式注入 - 字段类型探测";
                                        break;
                                    }
                                    continue;
                                }
                                if (lt.equals(";") && i != tokens.length - 1
                                        && modules.containsKey(CONFIG_KEY_STACKED_QUERY)
                                        && modules.get(CONFIG_KEY_STACKED_QUERY)) {
                                    message = "禁止多语句查询";
                                    break;
                                } else if (lt.startsWith("0x")
                                        && modules.containsKey(CONFIG_KEY_NO_HEX)
                                        && modules.get(CONFIG_KEY_NO_HEX)) {
                                    message = "禁止16进制字符串";
                                    break;
                                } else if (lt.startsWith("/*!")
                                        && modules.containsKey(CONFIG_KEY_VERSION_COMMENT)
                                        && modules.get(CONFIG_KEY_VERSION_COMMENT)) {
                                    message = "禁止MySQL版本号注释";
                                    break;
                                } else if (i > 0 && i < tokens.length - 1 && (lt.equals("xor")
                                        || lt.charAt(0) == '<'
                                        || lt.charAt(0) == '>'
                                        || lt.charAt(0) == '=')
                                        && modules.containsKey(CONFIG_KEY_CONSTANT_COMPARE)
                                        && modules.get(CONFIG_KEY_CONSTANT_COMPARE)) {
                                    String op1 = tokens[i - 1];
                                    String op2 = tokens[i + 1];
                                    if (StringUtils.isNumeric(op1) && StringUtils.isNumeric(op2)) {
                                        try {
                                            if (Double.parseDouble(op1) < 10 || Double.parseDouble(op2) < 10) {
                                                continue;
                                            }
                                        } catch (Exception e) {
                                            // ignore
                                        }
                                        message = "禁止常量比较操作: " + op1 + " vs " + op2;
                                        break;
                                    }
                                } else if (i > 0 && tokens[i].indexOf('(') == 0
                                        && modules.containsKey(CONFIG_KEY_FUNCTION_BLACKLIST)
                                        && modules.get(CONFIG_KEY_FUNCTION_BLACKLIST)) {
                                    // FIXME: 可绕过，暂时不更新
                                    HashMap<String, Boolean> funBlackList = getJsonObjectAsMap(config, CONFIG_KEY_SQLI_POLICY, "function_blacklist");
                                    if (funBlackList.containsKey(tokens[i - 1]) && funBlackList.get(tokens[i - 1])) {
                                        message = "禁止执行敏感函数: " + tokens[i - 1];
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    if (message != null) {
                        result.add(AttackInfo.createLocalAttackInfo(checkParameter, action,
                                message, 100));
                    }
                }
            }
        } catch (Exception e) {
            JSContext.LOGGER.warn("An error occurred while the local sql plugin was detecting, because:" + e.getMessage());
        }

        // js 插件检测
        List<EventInfo> jsResults = new JsChecker().checkParam(checkParameter);
        if (jsResults != null && jsResults.size() > 0) {
            result.addAll(jsResults);
        }
        return result;
    }

}
