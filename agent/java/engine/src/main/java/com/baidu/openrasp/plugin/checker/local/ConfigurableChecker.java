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

import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.plugin.checker.AttackChecker;
import com.baidu.openrasp.plugin.js.JS;
import com.google.gson.Gson;
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.reflect.TypeToken;

import java.util.HashMap;

/**
 * Created by tyy on 17-12-21.
 *
 * 检测内容可以通过 json 配置
 */
public abstract class ConfigurableChecker extends AttackChecker {

    private static final int DEFAULT_MIN_LENGTH = -1;

    protected String getActionElement(JsonObject config, String key) {
        return getStringElement(config, key, "action");
    }

    protected JsonArray getJsonObjectAsArray(JsonObject config, String key, String subKey) {
        JsonArray result = null;
        try {
            JsonElement value = getElement(config, key, subKey);
            if (value != null) {
                result = value.getAsJsonArray();
            }
        } catch (Exception e) {
            logJsonError(e);
        }
        return result;
    }

    protected HashMap<String, Boolean> getJsonObjectAsMap(JsonObject config, String key, String subKey) {
        HashMap<String, Boolean> result = null;
        try {
            JsonElement value = getElement(config, key, subKey);
            if (value != null) {
                Gson gson = new Gson();
                result = gson.fromJson(value, new TypeToken<HashMap<String, Boolean>>() {
                }.getType());
            }
        } catch (Exception e) {
            logJsonError(e);
        }
        if (result == null) {
            result = new HashMap<String, Boolean>();
        }
        return result;
    }

    protected HashMap<String, Integer> getJsonObjectAsIntMap(JsonObject config, String key, String subKey) {
        HashMap<String, Integer> result = null;
        try {
            JsonElement value = getElement(config, key, subKey);
            if (value != null) {
                Gson gson = new Gson();
                result = gson.fromJson(value, new TypeToken<HashMap<String, Integer>>() {
                }.getType());
            }
        } catch (Exception e) {
            logJsonError(e);
        }
        if (result == null) {
            result = new HashMap<String, Integer>();
        }
        return result;
    }

    protected String getStringElement(JsonObject config, String key, String subKey) {
        try {
            JsonElement value = getElement(config, key, subKey);
            if (value != null) {
                return value.getAsString();
            }
        } catch (Exception e) {
            logJsonError(e);
        }
        return null;
    }

    private JsonElement getElement(JsonObject config, String key, String subKey) {
        if (config != null) {
            JsonElement jsonElement = config.get(key);
            if (jsonElement != null) {
                JsonElement value = jsonElement.getAsJsonObject().get(subKey);
                if (value != null) {
                    return value;
                }
            }
        }
        return null;
    }

    protected int getIntElement(JsonObject config, String key, String subKey) {

        try {
            JsonElement element = getElement(config, key, subKey);
            return element != null ? element.getAsInt() : DEFAULT_MIN_LENGTH;
        } catch (Exception e) {
            logJsonError(e);
        }
        return DEFAULT_MIN_LENGTH;
    }


    private void logJsonError(Exception e) {
        String message = "Parse json failed because: " + e.getMessage() +
                System.getProperty("line.separator") +
                "        Please check algorithmConfig in js";
        int errorCode = ErrorType.PLUGIN_ERROR.getCode();
        JS.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
    }

}
