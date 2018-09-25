package com.baidu.openrasp.cloud;

import com.baidu.openrasp.cloud.model.CloudRequestUrl;
import com.baidu.openrasp.cloud.model.GenericResponse;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.tool.OSUtil;
import com.baidu.openrasp.tool.model.ApplicationModel;
import com.google.gson.Gson;
import org.apache.log4j.Logger;

import java.io.File;
import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.Map;

/**
 * @description: 初始化云控配置
 * @author: anyang
 * @create: 2018/09/18 15:09
 */
public class CloudManager {
    private static final Logger LOGGER = Logger.getLogger(CloudManager.class.getPackage().getName() + ".log");

    public synchronized static void init(String projectVersion) {
        if (checkEnter()) {
            new KeepAlive();
            register(projectVersion);
        }
    }

    private static void register(String projectVersion) {
        try {
            Map<String, Object> params = GenerateParameters(projectVersion);
            String content = new Gson().toJson(params);
            String url = CloudRequestUrl.CLOUD_REGISTER_URL;
            String jsonString = new CloudHttp().request(url, content);
            GenericResponse response = new Gson().fromJson(jsonString, GenericResponse.class);
            if (response.getStatus() == 0 && "ok".equals(response.getDescription().toLowerCase())) {
                System.out.println("[OpenRASP] Cloud Control Registered Successed");
            } else {
                LOGGER.warn("Cloud control registered failed");
            }
        } catch (NoSuchAlgorithmException e) {
            LOGGER.error("get id failed: " + e.getMessage(), e);
        }
    }

    private static Map<String, Object> GenerateParameters(String projectVersion) throws NoSuchAlgorithmException {
        Map<String, Object> params = new HashMap<String, Object>();
        params.put("id", OSUtil.getID());
        params.put("version", projectVersion);
        params.put("host_name", OSUtil.getHostName());
        params.put("language", "java");
        params.put("server_type", ApplicationModel.getServerName());
        params.put("server_version", ApplicationModel.getVersion());
        String raspHome = new File(Config.getConfig().getBaseDirectory()).getParent();
        params.put("rasp_home", raspHome);
        return params;
    }

    private static boolean checkEnter() {
        if (Config.getConfig().getCloudSwitch()) {
            return !Config.getConfig().getCloudAddress().isEmpty() &&
                    !Config.getConfig().getCloudAddress().isEmpty();
        }
        return false;
    }
}
