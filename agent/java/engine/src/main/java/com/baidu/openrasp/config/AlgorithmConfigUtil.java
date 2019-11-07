package com.baidu.openrasp.config;

import com.baidu.openrasp.plugin.checker.local.ConfigurableChecker;
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import org.apache.commons.lang3.StringUtils;

import java.util.HashMap;
import java.util.HashSet;

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
        JsonArray result = null;
        JsonElement elements = ConfigurableChecker.getElement(Config.getConfig().algorithmConfig,
                "sql_exception", "mysql");
        if (elements != null) {
            JsonElement e = elements.getAsJsonObject().get("error_code");
            if (e != null) {
                result = e.getAsJsonArray();
            }
        }
        HashSet<Integer> errorCodes = new HashSet<Integer>();
        if (result != null) {
            if (result.size() > Config.MAX_SQL_EXCEPTION_CODES_COUNT) {
                Config.LOGGER.warn("size of RASP.algorithmConfig.sql_exception.error_code can not be greater than "
                        + Config.MAX_SQL_EXCEPTION_CODES_COUNT);
            } else {
                for (JsonElement element : result) {
                    try {
                        errorCodes.add(element.getAsInt());
                    } catch (Exception e) {
                        Config.LOGGER.warn("failed to add a json error code element: "
                                + element.toString() + ", " + e.getMessage(), e);
                    }
                }
            }
        } else {
            Config.LOGGER.warn(
                    "failed to get the sql_exception.${DB_TYPE}.error_code element from algorithm config");
        }
        Config.getConfig().sqlErrorCodes = errorCodes;
        Config.LOGGER.info("sql error codes: " + Config.getConfig().sqlErrorCodes.toString());
    }

}
