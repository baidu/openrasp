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
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;

import java.util.HashSet;

/**
 * Created by tyy on 17-12-21.
 *
 * 检测内容可以通过 json 配置
 */
public abstract class ConfigurableChecker extends AttackChecker {

    protected String getActionElement(JsonObject config, String key) {
        return getStringElement(config, key, "action");
    }

    protected HashSet<String> getJsonArrayAsSet(JsonObject config, String key, String subKey) {
        HashSet<String> set = new HashSet<String>(8);
        try {
            JsonElement value = getElement(config, key, subKey);
            if (value != null) {
                JsonArray jsonArray = value.getAsJsonArray();
                if (jsonArray != null) {
                    for (JsonElement element : jsonArray) {
                        if (element != null) {
                            String module = element.getAsString();
                            if (module != null) {
                                set.add(module);
                            }
                        }
                    }
                }
            }
        } catch (Exception e) {
            Config.LOGGER.warn("parse jason failed because: " + e.getMessage());
        }
        return set;
    }

    protected String getStringElement(JsonObject config, String key, String subKey) {
        try {
            JsonElement value = getElement(config, key, subKey);
            if (value != null) {
                return value.getAsString();
            }
        } catch (Exception e) {
            Config.LOGGER.warn("parse jason failed because: " + e.getMessage());
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

}
