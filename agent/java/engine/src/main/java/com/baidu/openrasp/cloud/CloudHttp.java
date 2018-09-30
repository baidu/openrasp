package com.baidu.openrasp.cloud;

import com.baidu.openrasp.cloud.Utils.CloudUtils;
import com.baidu.openrasp.cloud.model.GenericResponse;
import com.baidu.openrasp.config.Config;
import com.google.gson.*;
import com.google.gson.reflect.TypeToken;

import java.io.*;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLConnection;

/**
 * @description: 云控http请求
 * @author: anyang
 * @create: 2018/09/17 16:56
 */
public class CloudHttp {
    private static final int DEFAULT_CONNECTION_TIMEOUT = 10000;
    private static final int DEFAULT_READ_TIMEOUT = 10000;

    public GenericResponse request(String url, String content) {
        DataOutputStream out = null;
        InputStream in = null;
        String jsonString = null;
        int responseCode;
        try {
            URL realUrl = new URL(url);
            URLConnection conn = realUrl.openConnection();
            HttpURLConnection httpUrlConnection = (HttpURLConnection) conn;
            httpUrlConnection.setRequestProperty("Content-Type", "application/json");
            String appId = Config.getConfig().getCloudAppId();
            httpUrlConnection.setRequestProperty("X-OpenRASP-AppID", appId);
            httpUrlConnection.setConnectTimeout(DEFAULT_CONNECTION_TIMEOUT);
            httpUrlConnection.setReadTimeout(DEFAULT_READ_TIMEOUT);
            httpUrlConnection.setRequestMethod("POST");
            httpUrlConnection.setUseCaches(false);
            httpUrlConnection.setDoOutput(true);
            httpUrlConnection.setDoInput(true);
            out = new DataOutputStream(httpUrlConnection.getOutputStream());
            out.writeBytes(content);
            out.flush();
            httpUrlConnection.connect();
            responseCode = httpUrlConnection.getResponseCode();
            in = httpUrlConnection.getInputStream();
            jsonString = CloudUtils.convertInputStreamToJsonString(in);
        } catch (Exception e) {
            return null;
        } finally {
            try {
                if (out != null) {
                    out.close();
                }
                if (in != null) {
                    in.close();
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        Gson gson = CloudUtils.getResponseGsonObject();
        GenericResponse response = gson.fromJson(jsonString, new TypeToken<GenericResponse>() {}.getType());
        response.setResponseCode(responseCode);
        return response;
    }
}
