package com.baidu.openrasp.cloud.model;

        import com.baidu.openrasp.config.Config;

public interface CloudRequestUrl {
    String cloudAddress = Config.getConfig().getCloudAddress();
    String CLOUD_HEART_BEAT_URL = cloudAddress + "/v1/agent/heartbeat";
    String CLOUD_REGISTER_URL = cloudAddress + "/v1/agent/rasp";
    String CLOUD_ALARM_HTTP_APPENDER_URL = cloudAddress + "/v1/agent/log/attack";
    String CLOUD_POLICY_ALARM_HTTP_APPENDER_URL = cloudAddress + "v1/agent/log/policy";
    String CLOUD_PLUGIN_HTTP_APPENDER_URL = cloudAddress + "v1/agent/log/plugin";
}
