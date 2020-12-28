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

/**
 * Created by tyy on 19-10-22.
 */
public abstract class ConfigSetter<T> {

    String itemName;

    public ConfigSetter(String itemName) {
        this.itemName = itemName;
    }

    abstract void setValue(T value);

    boolean setDefaultValue() {
        try {
            setValue(getDefaultValue());
        } catch (Throwable t) {
            Config.LOGGER.warn("set default value of " + itemName + " failed:" + t.getMessage(), t);
            return false;
        }
        return true;
    }

    abstract T getDefaultValue();
}
