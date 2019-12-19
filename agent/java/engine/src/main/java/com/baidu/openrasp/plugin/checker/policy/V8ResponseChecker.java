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

package com.baidu.openrasp.plugin.checker.policy;

import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.checker.policy.PolicyChecker;
import com.baidu.openrasp.v8.ByteArrayOutputStream;
import com.baidu.openrasp.v8.V8;
import com.baidu.openrasp.plugin.js.JS;
import com.baidu.openrasp.plugin.js.Context;
import com.jsoniter.JsonIterator;
import com.jsoniter.any.Any;
import com.jsoniter.output.JsonStream;
import com.jsoniter.spi.TypeLiteral;
import com.jsoniter.ValueType;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.messaging.ErrorType;
import com.baidu.openrasp.messaging.LogTool;
import org.apache.log4j.Logger;
import com.baidu.openrasp.plugin.info.SecurityPolicyInfo;
import com.baidu.openrasp.plugin.info.EventInfo;
import java.util.List;

/**
 * Created by tyy on 19-12-5.
 *
 * V8 基线检测
 */
public class V8ResponseChecker extends PolicyChecker {
    public static final Logger PLUGIN_LOGGER = JS.PLUGIN_LOGGER;
    public static final Logger LOGGER = JS.LOGGER;

    public V8ResponseChecker() {
        super();
    }

    public V8ResponseChecker(boolean canBlock) {
        super(canBlock);
    }

    @Override
    public List<EventInfo> checkParam(CheckParameter checkParameter) {
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        JsonStream.serialize(checkParameter.getParams(), out);
        out.write(0);
        byte[] results = null;
        try {
            results = V8.Check(checkParameter.getType().getName(), out.getByteArray(), out.size(),
                    new Context(checkParameter.getRequest()), (int) Config.getConfig().getPluginTimeout());
        } catch (Exception e) {
            LogTool.error(ErrorType.PLUGIN_ERROR, e.getMessage(), e);
            return null;
        }
        try {
            Any any = JsonIterator.deserialize(results);
            if (any == null) {
                return null;
            }
            ArrayList<EventInfo> infos = new ArrayList<EventInfo>();
            for (Any rst : any.asList()) {
                String message = rst.toString("message");
                Any policy_params = rst.get("policy_params");
                Map<String, Object> params = null;
                if (policy_params.valueType() == ValueType.OBJECT) {
                    params = policy_params.as(new TypeLiteral<Map<String, Object>>() {
                    });
                } else {
                    params = new HashMap<String, Object>();
                }
                SecurityPolicyInfo info = new SecurityPolicyInfo(SecurityPolicyInfo.Type.SENSITIVE_OUTOUT, message,
                        false, params);
                infos.add(info);
            }
            return infos;
        } catch (Exception e) {
            LOGGER.warn(e);
            return null;
        }
    }
}
