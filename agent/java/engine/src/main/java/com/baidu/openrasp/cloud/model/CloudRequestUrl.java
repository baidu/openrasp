package com.baidu.openrasp.cloud.model;

        import com.baidu.openrasp.config.Config;

public interface CloudRequestUrl {
    String cloudAddress = Config.getConfig().getCloudAddress();
    String CLOUD_HEART_BEAT_URL = cloudAddress + "/v1/agent/heartbeat";
    String CLOUD_REHISTER_URL = cloudAddress + "/v1/agent/rasp";
    String CLOUD_ALARM_HTTP_APPENDER_URL = cloudAddress + "/v1/agent/log/attack";
}
