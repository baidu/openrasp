package com.baidu.openrasp.cloud.Utils;

import com.baidu.openrasp.cloud.model.CloudCacheModel;
import com.baidu.openrasp.cloud.model.GenericResponse;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.plugin.js.engine.JSContext;
import com.baidu.openrasp.tool.OSUtil;
import com.google.gson.*;
import com.google.gson.reflect.TypeToken;
import org.mozilla.javascript.ScriptableObject;

import java.io.*;
import java.lang.reflect.Type;
import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * @description: 插件工具类
 * @author: anyang
 * @create: 2018/09/17 17:40
 */
public class CloudUtils {
    public static String convertInputStreamToJsonString(InputStream inputStream) {
        String jsonString = null;
        try {
            ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
            byte[] data = new byte[1024];
            int len = 0;
            while ((len = inputStream.read(data)) != -1) {
                outputStream.write(data, 0, len);
            }
            jsonString = new String(outputStream.toByteArray(),"UTF-8");
            inputStream.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return jsonString;
    }

    public static Gson getResponseGsonObject() {
        return new GsonBuilder().registerTypeAdapter(
                new TypeToken<GenericResponse>() {
                }.getType(),
                new JsonDeserializer<GenericResponse>() {
                    public GenericResponse deserialize(JsonElement jsonElement, Type type, JsonDeserializationContext jsonDeserializationContext) throws JsonParseException {
                        GenericResponse response = new GenericResponse();
                        Map<String, Object> map = new HashMap<String, Object>();
                        JsonObject jsonObject = jsonElement.getAsJsonObject();
                        response.setDescription(jsonObject.get("description").getAsString());
                        response.setStatus(jsonObject.get("status").getAsInt());
                        Set<Map.Entry<String, JsonElement>> entrySet = jsonObject.get("data").getAsJsonObject().entrySet();
                        for (Map.Entry<String, JsonElement> entry : entrySet) {
                            map.put(entry.getKey(), entry.getValue());
                        }
                        response.setData(map);
                        return response;

                    }
                }
        ).create();
    }

    public static Gson getMapGsonObject() {
        return new GsonBuilder().registerTypeAdapter(
                new TypeToken<Map<String, Object>>() {
                }.getType(),
                new JsonDeserializer<Map<String, Object>>() {
                    public Map<String, Object> deserialize(JsonElement jsonElement, Type type, JsonDeserializationContext jsonDeserializationContext) throws JsonParseException {
                        Map<String, Object> map = new HashMap<String, Object>();
                        JsonObject jsonObject = jsonElement.getAsJsonObject();
                        Set<Map.Entry<String, JsonElement>> entrySet = jsonObject.getAsJsonObject().entrySet();
                        for (Map.Entry<String, JsonElement> entry : entrySet) {
                            map.put(entry.getKey(), entry.getValue());
                        }
                        return map;
                    }
                }
        ).create();
    }

    public static boolean checkCloudControlEnter() throws NoSuchAlgorithmException {
        CloudCacheModel.getInstance().setRaspId(OSUtil.getID());
        if (Config.getConfig().getCloudSwitch()) {
            return !Config.getConfig().getCloudAddress().isEmpty() &&
                    !Config.getConfig().getCloudAppId().isEmpty();
        }
        return false;
    }

    public static Map<String, Object> getMapFromData(GenericResponse response, String key) {
        Map<String, Object> data = response.getData();
        if (data != null) {
            Object object = data.get(key);

            if (object != null) {
                JsonObject jsonElement = (JsonObject)object;
                return getMapGsonObject().fromJson(jsonElement,new TypeToken<Map<String,Object>>(){}.getType());
            }
        }
        return null;
    }

    public static Object getValueFromData(GenericResponse response, String key) {
        Map<String, Object> data = response.getData();
        if (data != null) {
            return data.get(key);
        }
        return null;
    }
}
