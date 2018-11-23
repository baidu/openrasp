/*
 * Copyright 2017-2018 Baidu Inc.
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

package com.baidu.openrasp.cloud;

import com.baidu.openrasp.cloud.utils.CloudUtils;
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
            String appSecret = Config.getConfig().getCloudAppSecret();
            httpUrlConnection.setRequestProperty("X-OpenRASP-AppSecret",appSecret);
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
        } catch (IOException e) {
            CloudManager.LOGGER.warn("HTTP request to " + url +" failed:", e);
            return null;
        } finally {
            try {
                if (out != null) {
                    out.close();
                }
                if (in != null) {
                    in.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        Gson gson = CloudUtils.getResponseGsonObject();
        GenericResponse response = gson.fromJson(jsonString, new TypeToken<GenericResponse>() {}.getType());
        response.setResponseCode(responseCode);
        return response;
    }
}
