package com.baidu.openrasp.cloud;

import com.baidu.openrasp.config.Config;
import com.google.gson.Gson;

import java.io.DataOutputStream;
import java.io.File;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLConnection;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

/**
 * @description: 云控http请求
 * @author: anyang
 * @create: 2018/09/17 16:56
 */
public class CloudHttp {
    private static final int DEFAULT_CONNECTION_TIMEOUT = 10000;
    private static final int DEFAULT_READ_TIMEOUT = 10000;

    public String request(String url, String content) {
        DataOutputStream out = null;
        InputStream in = null;
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
            in = httpUrlConnection.getInputStream();
            responseCode = httpUrlConnection.getResponseCode();
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
        Map<String,Object> res = new Gson().fromJson(new Gson().toJson(in),Map.class);
        res.put("responseCode",responseCode);
        return new Gson().toJson(res);

    }
}
