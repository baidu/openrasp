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
import com.baidu.openrasp.plugin.checker.AttackChecker;
import com.baidu.openrasp.plugin.js.engine.JSContext;
import com.google.gson.Gson;
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

    protected String getActionElement(JsonObject config, String key) {
        return getStringElement(config, key, "action");
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

    private void logJsonError(Exception e) {
        JSContext.LOGGER.warn("Parse jason failed because: " + e.getMessage() +
                System.getProperty("line.separator") +
                "        Please check algorithmConfig in js");
    }

}
