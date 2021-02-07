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

package com.baidu.openrasp.config;

import com.baidu.openrasp.hook.sql.AbstractSqlHook;
import com.baidu.openrasp.plugin.checker.local.ConfigurableChecker;
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import org.apache.commons.lang3.StringUtils;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import static com.baidu.openrasp.config.Config.MAX_LOG_REGEX_LENGTH;

/**
 * Created by tyy on 19-11-5.
 */
public class AlgorithmConfigUtil {

    static void setLogRegexes() {
        JsonArray regexArray = null;
        JsonElement elements = ConfigurableChecker.getElement(Config.getConfig().algorithmConfig,
                "log_regex", "regex");
        if (elements != null) {
            regexArray = elements.getAsJsonArray();
        }
        HashMap<String, String> regexData = new HashMap<String, String>();
        if (regexArray != null) {
            regexData = getRegexData(regexArray);
        } else {
            Config.LOGGER.warn("failed to get the log_regex.regex element from algorithm config");
        }
        Config.getConfig().logSensitiveRegex = regexData;
        Config.LOGGER.info("log_regex.regex: " + Config.getConfig().logSensitiveRegex.toString());
    }

    private static HashMap<String, String> getRegexData(JsonArray regexArray) {
        HashMap<String, String> regexData = new HashMap<String, String>();
        if (regexArray.size() > Config.MAX_LOG_REGEX_COUNT) {
            Config.LOGGER.warn("size of RASP.algorithmConfig.log_regex.regex can not be greater than "
                    + Config.MAX_LOG_REGEX_COUNT);
        } else {
            for (JsonElement element : regexArray) {
                try {
                    if (element == null) {
                        Config.LOGGER.warn("the each algorithmConfig.log_regex.regex can not be null");
                        continue;
                    }
                    JsonObject regex = element.getAsJsonObject();
                    JsonElement nameElement = regex.get("name");
                    JsonElement valueElement = regex.get("value");
                    if (nameElement != null && valueElement != null) {
                        String name = nameElement.getAsString();
                        String value = valueElement.getAsString();
                        if (StringUtils.isEmpty(name) || StringUtils.isEmpty(value)
                                || name.length() > MAX_LOG_REGEX_LENGTH || value.length() > MAX_LOG_REGEX_LENGTH) {
                            Config.LOGGER.warn("the length of name and value of each algorithmConfig.log_regex.regex" +
                                    " must be between [1,200], failed to set");
                        } else {
                            regexData.put(name, value);
                        }
                    } else {
                        Config.LOGGER.warn("the name and value of each algorithmConfig.log_regex.regex can not be empty");
                    }
                } catch (Exception e) {
                    Config.LOGGER.warn("failed to add a log regex element: "
                            + element.toString() + ", " + e.getMessage(), e);
                }
            }
        }
        return regexData;
    }

    static void setSqlErrorCodes() {
        Map<String, Set<String>> errorCodes = new HashMap<String, Set<String>>();
        Map<String, Set<String>> errorStates = new HashMap<String, Set<String>>();
        JsonElement elements = Config.getConfig().algorithmConfig.get("sql_exception");
        if (elements != null) {
            JsonObject sqlExceptionObject = elements.getAsJsonObject();
            for (AbstractSqlHook.SqlType sqlType : AbstractSqlHook.SqlType.values()) {
                JsonElement sqlElement = sqlExceptionObject.get(sqlType.name);
                if (sqlElement != null) {
                    Set<String> codes = getErrorsFromJson(sqlElement, "error_code", sqlType.name);
                    if (codes != null) {
                        errorCodes.put(sqlType.name, codes);
                    }
                    Set<String> states = getErrorsFromJson(sqlElement, "error_state", sqlType.name);
                    if (states != null) {
                        errorStates.put(sqlType.name, states);
                    }
                }
            }
        }
        Config.getConfig().sqlErrorCodes = errorCodes;
        Config.getConfig().sqlErrorStates = errorStates;
        Config.LOGGER.info("sql error codes: " + Config.getConfig().sqlErrorCodes.toString());
        Config.LOGGER.info("sql error states: " + Config.getConfig().sqlErrorStates.toString());
    }

    private static HashSet<String> getErrorsFromJson(JsonElement sqlElement, String property, String sqlType) {
        HashSet<String> codes = null;
        JsonObject sqlObject = sqlElement.getAsJsonObject();
        JsonElement errorElement = sqlObject.get(property);
        if (errorElement != null) {
            JsonArray errorArray = errorElement.getAsJsonArray();
            if (errorArray.size() > Config.MAX_SQL_EXCEPTION_CODES_COUNT) {
                Config.LOGGER.warn("size of sql_exception." + sqlType +
                        ".error_code can not be greater than " + Config.MAX_SQL_EXCEPTION_CODES_COUNT);
            } else {
                codes = new HashSet<String>();
                for (JsonElement element : errorArray) {
                    try {
                        String code = element.getAsString();
                        if (StringUtils.isEmpty(code) || code.length() > 200) {
                            Config.LOGGER.warn("the each element's length of sql_exception." +
                                    sqlType + ".error_code must be between [1,200]");
                        } else {
                            codes.add(element.getAsString());
                        }
                    } catch (Exception e) {
                        Config.LOGGER.warn("failed to add a json error code element: " +
                                element.toString() + ", " + e.getMessage(), e);
                    }
                }

            }
        } else {
            Config.LOGGER.warn("the value of sql_exception." + sqlType + ".error_code is null");
        }
        return codes;
    }

}
