package com.baidu.openrasp.cloud.Utils;

import com.baidu.openrasp.cloud.model.GenericResponse;
import com.baidu.openrasp.config.Config;
import com.google.gson.Gson;

import java.io.*;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * @description: 插件工具类
 * @author: anyang
 * @create: 2018/09/17 17:40
 */
public class CloudUtils {
    public static String getPluginVersion(String baseDir) {
        String pluginPath = baseDir + File.separator + "plugins" + File.separator + "official.js";
        File file = new File(pluginPath);
        String pluginVersion = null;
        if (file.exists()) {
            try {
                BufferedReader reader = new BufferedReader(new InputStreamReader(new FileInputStream(file)));
                String line;
                while ((line = reader.readLine()) != null) {
                    if (line.toLowerCase().contains("version")) {
                        String regex = "'([^']*)'";
                        Matcher matcher = Pattern.compile(regex).matcher(line);
                        if (matcher.find()) {
                            pluginVersion = matcher.group(0).replaceAll(regex, "$1");
                        }
                        break;
                    }
                }
            } catch (IOException e) {
                Config.LOGGER.warn("plugin version set failed", e);
            }
        }

        return pluginVersion != null ? pluginVersion : "";
    }

    public static Map<String,Object> getMapFromResponse(GenericResponse response,String key){
        Object object = response.getData().get(key);
        if (object!=null){
            return new Gson().fromJson(new Gson().toJson(object),Map.class);
        }
        return null;
    }
}
