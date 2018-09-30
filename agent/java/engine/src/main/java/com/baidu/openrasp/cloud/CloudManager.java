package com.baidu.openrasp.cloud;

import com.baidu.openrasp.cloud.Utils.CloudUtils;
import com.baidu.openrasp.cloud.model.CloudCacheModel;
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
    public static final Logger LOGGER = Logger.getLogger(CloudManager.class.getPackage().getName() + ".log");

    public static void init(String projectVersion) throws Exception {
        if (CloudUtils.checkCloudControlEnter()) {
            String content = new Gson().toJson(KeepAlive.GenerateParameters());
            String url = CloudRequestUrl.CLOUD_HEART_BEAT_URL;
            GenericResponse response = new CloudHttp().request(url, content);
            if (response != null && response.getStatus() != null && response.getStatus() == 0) {
                new KeepAlive();
                register(projectVersion);
                new StatisticsReport();
            } else {
                System.out.println("[OpenRASP] Cloud Control Send KeepAlive Failed");
                throw new Exception();
            }

        }
    }

    private static void register(String projectVersion) throws Exception {

        Map<String, Object> params = GenerateParameters(projectVersion);
        String content = new Gson().toJson(params);
        String url = CloudRequestUrl.CLOUD_REGISTER_URL;
        GenericResponse response = new CloudHttp().request(url, content);
        if (response != null) {
            if (response.getStatus() != null && response.getStatus() == 0) {
                System.out.println("[OpenRASP] Cloud Control Registered Successed");
            } else {
                System.out.println("[OpenRASP] Cloud Control Registered Failed");
                throw new Exception();
            }
        }
    }

    private static Map<String, Object> GenerateParameters(String projectVersion) {
        Map<String, Object> params = new HashMap<String, Object>();
        params.put("id", CloudCacheModel.getInstance().getRaspId());
        params.put("version", projectVersion);
        params.put("host_name", OSUtil.getHostName());
        params.put("language", "java");
        params.put("language_version", System.getProperty("java.version"));
        params.put("server_type", ApplicationModel.getServerName());
        params.put("server_version", ApplicationModel.getVersion());
        String raspHome = new File(Config.getConfig().getBaseDirectory()).getParent();
        params.put("rasp_home", raspHome);
        return params;
    }

}
