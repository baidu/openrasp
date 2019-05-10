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

package com.baidu.openrasp.hook.xxe;

import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;

/**
 * @description: 关闭XML的外部实体引用
 * @author: anyang
 * @create: 2019/04/30 14:48
 */
public abstract class DisableXxeEntity extends AbstractClassHook {

    @Override
    public String getType() {
        return "xxe";
    }

    private static final String XXE_DISABLE_ENTITY = "xxe_disable_entity";
    private static final String DEFAULT_XXE_DISABLE_ENTITY = "ignore";
    public static final String BLOCK_XXE_DISABLE_ENTITY = "block";
    public static final String FEATURE = "http://apache.org/xml/features/disallow-doctype-decl";

    protected static String getAction() {
        try {
            JsonObject config = Config.getConfig().getAlgorithmConfig();
            if (config != null) {
                JsonElement jsonElement = config.get(XXE_DISABLE_ENTITY);
                if (jsonElement != null) {
                    JsonElement value = jsonElement.getAsJsonObject().get("action");
                    if (value != null) {
                        return value.getAsString();
                    }
                }
            }
        } catch (Exception e) {
            return DEFAULT_XXE_DISABLE_ENTITY;
        }
        return DEFAULT_XXE_DISABLE_ENTITY;
    }

    protected static boolean getStatus(String type) {
        try {
            JsonObject config = Config.getConfig().getAlgorithmConfig();
            if (config != null) {
                JsonElement jsonElement = config.get(XXE_DISABLE_ENTITY);
                if (jsonElement != null) {
                    JsonElement value = jsonElement.getAsJsonObject().get("clazz");
                    if (value != null) {
                        JsonElement status = value.getAsJsonObject().get(type);
                        if (status != null) {
                            return status.getAsBoolean();
                        }
                    }
                }
            }
        } catch (Exception e) {
            return false;
        }
        return false;
    }
}
