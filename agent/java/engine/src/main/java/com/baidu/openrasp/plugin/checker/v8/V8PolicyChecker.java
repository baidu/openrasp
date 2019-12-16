package com.baidu.openrasp.plugin.checker.v8;

import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.checker.policy.PolicyChecker;
import com.baidu.openrasp.v8.ByteArrayOutputStream;
import com.baidu.openrasp.v8.V8;
import com.baidu.openrasp.plugin.js.JS;
import com.baidu.openrasp.plugin.js.Context;
import com.jsoniter.JsonIterator;
import com.jsoniter.any.Any;
import com.jsoniter.output.JsonStream;
import java.util.ArrayList;
import java.util.HashMap;

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
public class V8PolicyChecker extends PolicyChecker {
    public static final Logger PLUGIN_LOGGER = JS.PLUGIN_LOGGER;
    public static final Logger LOGGER = JS.LOGGER;

    public V8PolicyChecker() {
        super();
    }

    public V8PolicyChecker(boolean canBlock) {
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
                    new Context(checkParameter.getRequest()), false, (int) Config.getConfig().getPluginTimeout());
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
                SecurityPolicyInfo info = new SecurityPolicyInfo(SecurityPolicyInfo.Type.SENSITIVE_OUTOUT,
                        rst.toString("message"), false);
                infos.add(info);
            }
            return infos;
        } catch (Exception e) {
            LOGGER.warn(e);
            return null;
        }
    }
}
