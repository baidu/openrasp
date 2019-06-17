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

package com.baidu.openrasp.cloud.utils;

import com.baidu.openrasp.cloud.CloudManager;
import com.baidu.openrasp.cloud.model.CloudCacheModel;
import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.model.ExceptionModel;
import com.baidu.openrasp.cloud.model.GenericResponse;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.tool.OSUtil;
import com.google.gson.*;
import com.google.gson.reflect.TypeToken;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.lang.reflect.Type;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

/**
 * @description: 云控工具类
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
            jsonString = new String(outputStream.toByteArray(), "UTF-8");
            inputStream.close();
        } catch (IOException e) {
            CloudManager.LOGGER.info("convert inputStream to json string failed");
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
                        JsonElement status = jsonObject.get("status");
                        JsonElement description = jsonObject.get("description");
                        JsonElement data = jsonObject.get("data");
                        if (status != null) {
                            response.setStatus(jsonObject.get("status").getAsInt());
                        }
                        if (description != null) {
                            response.setDescription(jsonObject.get("description").getAsString());
                        }
                        if (data != null) {
                            Set<Map.Entry<String, JsonElement>> entrySet = jsonObject.get("data").getAsJsonObject().entrySet();
                            for (Map.Entry<String, JsonElement> entry : entrySet) {
                                map.put(entry.getKey(), entry.getValue());
                            }
                            response.setData(map);
                        }
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
                        if (jsonObject != null) {
                            Set<Map.Entry<String, JsonElement>> entrySet = jsonObject.getAsJsonObject().entrySet();
                            for (Map.Entry<String, JsonElement> entry : entrySet) {
                                map.put(entry.getKey(), entry.getValue());
                            }
                        }
                        return map;
                    }
                }
        ).create();
    }

    public static boolean checkCloudControlEnter() {
        if (Config.getConfig().getCloudSwitch()) {
            try {
                CloudCacheModel.getInstance().setRaspId(OSUtil.getRaspId());
            } catch (Exception e) {
                String message = "Unable to generate unique rasp_id";
                int errorCode = ErrorType.CONFIG_ERROR.getCode();
                CloudManager.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
                return false;
            }

            String cloudAddress = Config.getConfig().getCloudAddress();
            String cloudAppId = Config.getConfig().getCloudAppId();
            String cloudAppSecret = Config.getConfig().getCloudAppSecret();
            if (cloudAddress == null || cloudAddress.trim().isEmpty()) {
                String message = "Cloud control configuration error: cloud.address is not configured";
                int errorCode = ErrorType.CONFIG_ERROR.getCode();
                CloudManager.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode));
                return false;
            }
            if (cloudAppId == null || cloudAppId.trim().isEmpty()) {
                String message = "Cloud control configuration error: cloud.appid is not configured";
                int errorCode = ErrorType.CONFIG_ERROR.getCode();
                CloudManager.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode));
                return false;
            }
            if (cloudAppSecret == null || cloudAppSecret.trim().isEmpty()) {
                String message = "Cloud control configuration error: cloud.appsecret is not configured";
                int errorCode = ErrorType.CONFIG_ERROR.getCode();
                CloudManager.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode));
                return false;
            }
            return true;
        }
        return false;
    }

    public static boolean checkRequestResult(GenericResponse response) {
        if (response != null) {
            if (Config.getConfig().isDebugEnabled()) {
                CloudManager.LOGGER.info(response.toString());
            }
            return response.getResponseCode() != null && response.getResponseCode() >= 200 &&
                    response.getResponseCode() < 300 && response.getStatus() != null && response.getStatus() == 0;
        } else {
            if (Config.getConfig().isDebugEnabled()) {
                CloudManager.LOGGER.info("http request failed");
            }
        }
        return false;
    }

    public static Map<String, Object> getMapFromData(GenericResponse response, String key) {
        Map<String, Object> data = response.getData();
        if (data != null) {
            Object object = data.get(key);
            if (object instanceof JsonObject) {
                JsonObject jsonElement = (JsonObject) object;
                return getMapGsonObject().fromJson(jsonElement, new TypeToken<Map<String, Object>>() {
                }.getType());
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

    public static String getMD5(String originalString) throws NoSuchAlgorithmException, UnsupportedEncodingException {
        char[] hexArray = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
        MessageDigest md = MessageDigest.getInstance("MD5");
        md.update(originalString.getBytes("UTF-8"));
        byte[] byteArray = md.digest();
        char[] resultCharArray = new char[byteArray.length * 2];
        int index = 0;
        for (byte b : byteArray) {
            resultCharArray[index++] = hexArray[b >>> 4 & 0xf];
            resultCharArray[index++] = hexArray[b & 0xf];
        }
        return new String(resultCharArray);
    }

    public static String handleError(ErrorType errorType, GenericResponse response) {
        if (response == null) {
            return errorType.toString();
        }
        Integer statusCode = response.getResponseCode();
        String description = response.getDescription();
        String message = getFormattedMessage(statusCode, description);
        if (message.isEmpty()) {
            return errorType.toString();
        }
        return errorType.toString() + "," + message;
    }

    private static String getFormattedMessage(Integer code, String message) {
        String result = "";
        if (code != null) {
            result = result + "Status Code: " + code;
        }
        if (message != null) {
            result = result + " Description: " + message;
        }
        return result.trim();
    }

    public static ExceptionModel getExceptionObject(String message, int errorCode) {
        ExceptionModel model = new ExceptionModel();
        model.setMessage(message);
        model.setErrorCode(errorCode);
        return model;
    }
}
