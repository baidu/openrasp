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

package com.baidu.openrasp.plugin.info;

import com.baidu.openrasp.config.Config;
import com.google.gson.Gson;

import java.util.Arrays;
import java.util.Map;

/**
 * 报警事件信息类
 */
public abstract class EventInfo {

    public static final String CHECK_ACTION_BLOCK = "block";
    public static final String CHECK_ACTION_IGNORE = "ignore";
    public static final String CHECK_ACTION_INFO = "log";

    private String json;

    private boolean isBlock = false;

    public abstract String getType();

    public abstract Map<String, Object> getInfo();

    public boolean isBlock() {
        return isBlock;
    }

    public void setBlock(boolean block) {
        isBlock = block;
    }

    @Override
    public String toString() {
        if (json == null) {
            Map<String, Object> info = getInfo();
            json = new Gson().toJson(info);
        }
        return json;
    }

    protected StackTraceElement[] filter(StackTraceElement[] trace) {
        int i = 0;
        // 去除插件本身调用栈
        while (trace[i].getClassName().startsWith("com.baidu.openrasp") && i < trace.length) {
            i++;
        }
        return Arrays.copyOfRange(trace, i, Math.min(i + Config.getConfig().getLogMaxStackSize(), trace.length));
    }

    protected String stringify(StackTraceElement[] trace) {
        StringBuilder ret = new StringBuilder();
        for (int i = 0; i < trace.length; i++) {
            ret.append(trace[i].toString());
            ret.append("\n");
        }
        return ret.toString();
    }

}
