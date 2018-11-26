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
import com.baidu.openrasp.plugin.antlr.TokenGenerator;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.hook.sql.SQLStatementHook;
import com.baidu.openrasp.plugin.antlr.TokenResult;
import com.baidu.openrasp.plugin.antlr.TokenizeErrorListener;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.checker.js.JsChecker;
import com.baidu.openrasp.plugin.info.AttackInfo;
import com.baidu.openrasp.plugin.info.EventInfo;
import com.baidu.openrasp.plugin.js.engine.JSContext;
import com.baidu.openrasp.tool.JsonStringify;
import com.google.gson.JsonObject;
import org.apache.commons.lang3.StringUtils;
import org.antlr.v4.runtime.Token;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.ArrayList;

/**
 * Created by tyy on 17-12-20.
 * <p>
 * 检测 sql 语句的 java 版本
 */
public class SqlStatementChecker extends ConfigurableChecker {

    private static final String CONFIG_KEY_SQL_USER_INPUT = "sql_userinput";
    private static final String CONFIG_KEY_DB_MANAGER = "sqli_dbmanager";
    private static final String CONFIG_KEY_SQL_POLICY = "sql_policy";
    private static final String CONFIG_KEY_STACKED_QUERY = "stacked_query";
    private static final String CONFIG_KEY_NO_HEX = "no_hex";
    private static final String CONFIG_KEY_INFORMATION_SCHEMA = "information_schema";
    private static final String CONFIG_KEY_VERSION_COMMENT = "version_comment";
    private static final String CONFIG_KEY_FUNCTION_BLACKLIST = "function_blacklist";
    private static final String CONFIG_KEY_UNION_NULL = "union_null";
    private static final String CONFIG_KEY_INTO_OUTFILE = "into_outfile";
    private static final String CONFIG_KEY_MIN_LENGTH = "min_length";

    public List<EventInfo> checkSql(CheckParameter checkParameter, Map<String, String[]> parameterMap, JsonObject config) {
        List<EventInfo> result = new LinkedList<EventInfo>();
        String query = (String) checkParameter.getParam("query");
        String message = null;
        ArrayList<TokenResult> rawTokens = TokenGenerator.detailTokenize(query, new TokenizeErrorListener());
        String[] tokens = new String[rawTokens.size()];
        for (int j = 0; j < rawTokens.size(); j++) {
            tokens[j] = rawTokens.get(j).getText();
        }
        // 算法1: 匹配用户输入
        // 1. 简单识别逻辑是否发生改变
        String action = getActionElement(config, CONFIG_KEY_SQL_USER_INPUT);
        int paramterMinLength = getIntElement(config, CONFIG_KEY_SQL_USER_INPUT, CONFIG_KEY_MIN_LENGTH);

        if (!EventInfo.CHECK_ACTION_IGNORE.equals(action) && action != null && parameterMap != null) {
            for (Map.Entry<String, String[]> entry : parameterMap.entrySet()) {
                String[] v = entry.getValue();
                String value = v[0];
                if (value.length() <= paramterMinLength) {
                    continue;
                }

                // 简单识别用户输入
                int para_index = query.indexOf(value);
                if (para_index < 0) {
                    continue;
                }

                // 当用户输入穿越了2个token，就可以判定为SQL注入
                int start = tokens.length, end = tokens.length, distance = 2;

                // 寻找 token 起始点
                for (int i = 0; i < tokens.length; i++) {
                    if (rawTokens.get(i).getStop() >= para_index) {
                        start = i;
                        break;
                    }
                }

                // 寻找 token 结束点
                // 另外，最多需要遍历 distance 个 token
                for (int i = start; i < start + distance && i < tokens.length; i++) {
                    if (rawTokens.get(i).getStop() >= para_index + value.length() - 1) {
                        end = i;
                        break;
                    }
                }

                if (end - start > distance) {
                    message = "SQLi - SQL query structure altered by user input, request parameter name: " + entry.getKey();
                }
            }
        }
        if (message != null) {
            result.add(AttackInfo.createLocalAttackInfo(checkParameter, action,
                    message, "sql_userinput", 90));
        } else {
            // 算法2: SQL语句策略检查（模拟SQL防火墙功能）
            HashMap<String, Boolean> funcBlackList = getJsonObjectAsMap(config, CONFIG_KEY_SQL_POLICY, "function_blacklist");

            action = getActionElement(config, CONFIG_KEY_SQL_POLICY);
            if (!EventInfo.CHECK_ACTION_IGNORE.equals(action)) {
                int i = -1;
                if (tokens != null) {
                    HashMap<String, Boolean> modules = getJsonObjectAsMap(config, CONFIG_KEY_SQL_POLICY, "feature");

                    // token 转换小写
                    for (int z = 0; z < tokens.length; z++) {
                        tokens[z] = tokens[z].toLowerCase();
                    }

                    for (String token : tokens) {
                        i++;
                        if (!StringUtils.isEmpty(token)) {
                            if (token.equals("select")
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
                                    message = "SQLi - Detected UNION-NULL phrase in sql query";
                                    break;
                                }
                                continue;
                            }
                            if (token.equals(";") && i != tokens.length - 1
                                    && modules.containsKey(CONFIG_KEY_STACKED_QUERY)
                                    && modules.get(CONFIG_KEY_STACKED_QUERY)) {
                                message = "SQLi - Detected stacked queries";
                                break;
                            } else if (token.startsWith("0x")
                                    && modules.containsKey(CONFIG_KEY_NO_HEX)
                                    && modules.get(CONFIG_KEY_NO_HEX)) {
                                message = "SQLi - Detected hexadecimal values in sql query";
                                break;
                            } else if (token.startsWith("/*!")
                                    && modules.containsKey(CONFIG_KEY_VERSION_COMMENT)
                                    && modules.get(CONFIG_KEY_VERSION_COMMENT)) {
                                message = "SQLi - Detected MySQL version comment in sql query";
                                break;
                            } else if (i > 0 && tokens[i].indexOf('(') == 0
                                    && modules.containsKey(CONFIG_KEY_FUNCTION_BLACKLIST)
                                    && modules.get(CONFIG_KEY_FUNCTION_BLACKLIST)) {
                                // FIXME: 可绕过，暂时不更新                                
                                if (funcBlackList.containsKey(tokens[i - 1]) && funcBlackList.get(tokens[i - 1])) {
                                    message = "SQLi - Detected dangerous method call " + tokens[i - 1] + "() in sql query";
                                    break;
                                }
                            } else if (i < tokens.length - 2 && tokens[i].equals("into")
                                    && (tokens[i + 1].equals("outfile") || tokens[i + 1].equals("dumpfile"))
                                    && modules.containsKey(CONFIG_KEY_INTO_OUTFILE)
                                    && modules.get(CONFIG_KEY_INTO_OUTFILE)) {
                                message = "SQLi - Detected INTO OUTFILE phrase in sql query";
                                break;
                            } else if (i < tokens.length - 1 && tokens[i].equals("from")
                                    && modules.containsKey(CONFIG_KEY_INFORMATION_SCHEMA)
                                    && modules.get(CONFIG_KEY_INFORMATION_SCHEMA)) {
                                // 处理反引号和空格
                                String[] parts = tokens[i + 1].replace("`", "").split("\\.");
                                if (parts.length == 2) {
                                    String db = parts[0].trim();
                                    String table = parts[1].trim();
                                    if (db.equals("information_schema") && table.equals("tables")) {
                                        message = "SQLi - Detected access to MySQL information_schema.tables table";
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                if (message != null) {
                    result.add(AttackInfo.createLocalAttackInfo(checkParameter, action,
                            message, "sql_policy", 100));
                }
            }
        }
        return result;
    }

    @Override
    public List<EventInfo> checkParam(CheckParameter checkParameter) {
        List<EventInfo> result = new LinkedList<EventInfo>();
        JsonObject config = Config.getConfig().getAlgorithmConfig();
        Map<String, String[]> parameterMap = HookHandler.requestCache.get().getParameterMap();
        try {
            result = checkSql(checkParameter, parameterMap, config);
        } catch (Exception e) {
            JSContext.LOGGER.warn("Exception while running builtin sqli plugin: " + e.getMessage());
        }

        // js 插件检测
        List<EventInfo> jsResults = new JsChecker().checkParam(checkParameter);
        if (jsResults != null && jsResults.size() > 0) {
            result.addAll(jsResults);
        }
        // 检测无威胁的sql加入sql缓存
        if (result.isEmpty()) {
            String query = (String) checkParameter.getParam("query");
            if (SQLStatementHook.sqlCache.maxSize() != 0) {
                SQLStatementHook.sqlCache.put(query.trim(), null);
            }
        }
        return result;
    }

}
