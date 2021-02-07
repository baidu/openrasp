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

package com.baidu.openrasp.plugin.info;

import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.messaging.ErrorType;
import com.baidu.openrasp.messaging.LogTool;
import com.baidu.openrasp.request.AbstractRequest;
import com.google.gson.Gson;

import java.util.Arrays;
import java.util.Enumeration;
import java.util.HashMap;
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
        try {
            if (json == null) {
                Map<String, Object> info = getInfo();
                json = new Gson().toJson(info);
            }
            return json;
        } catch (Throwable t) {
            LogTool.error(ErrorType.HOOK_ERROR, "failed to print event log: " + t.getMessage(), t);
            return null;
        }
    }

    protected String stringify(StackTraceElement[] trace) {
        StringBuilder ret = new StringBuilder();
        for (int i = 0; i < trace.length; i++) {
            ret.append(trace[i].toString());
            ret.append("\n");
        }
        return ret.toString();
    }

    protected Map<String, String> getRequestHeader(AbstractRequest request) {
        Map<String, String> header = new HashMap<String, String>();
        if (request != null) {
            Enumeration<String> headerNames = request.getHeaderNames();
            if (headerNames != null) {
                while (headerNames.hasMoreElements()) {
                    String key = headerNames.nextElement();
                    String value = request.getHeader(key);
                    header.put(key.toLowerCase(), value);
                }
            }
        }
        return header;
    }

}
