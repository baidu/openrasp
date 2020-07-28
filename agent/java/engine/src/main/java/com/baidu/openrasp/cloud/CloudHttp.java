/*
 * Copyright 2017-2020 Baidu Inc.
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

import com.baidu.openrasp.cloud.model.CloudRequestUrl;
import com.baidu.openrasp.cloud.model.GenericResponse;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.messaging.ErrorType;
import com.baidu.openrasp.messaging.LogTool;
import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;

import javax.net.ssl.*;
import java.io.DataOutputStream;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;
import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.zip.GZIPInputStream;

/**
 * @description: 云控http请求
 * @author: anyang
 * @create: 2018/09/17 16:56
 */
public class CloudHttp implements Request {
    private static final int DEFAULT_CONNECTION_TIMEOUT = 10000;
    private static final int DEFAULT_READ_TIMEOUT = 10000;

    @Override
    public GenericResponse commonRequest(String url, String content) {
        try {
            return request(url, content);
        } catch (Exception e) {
            LogTool.warn(ErrorType.REQUEST_ERROR, "HTTP request to " + url + " failed: " + e.getMessage(), e);
            return null;
        }
    }

    @Override
    public GenericResponse logRequest(String url, String content) {
        try {
            return request(url, content);
        } catch (Exception e) {
            if (!CloudRequestUrl.CLOUD_EXCEPTION_HTTP_APPENDER_URL.equals(url)) {
                LogTool.warn(ErrorType.REQUEST_ERROR, "HTTP request to " + url + " failed: " + e.getMessage(), e);
            }
            return null;
        }
    }

    public GenericResponse request(String url, String content) throws Exception {
        DataOutputStream out = null;
        InputStream in = null;
        String jsonString = null;
        int responseCode;
        try {
            URL realUrl = new URL(url);
            URLConnection conn = realUrl.openConnection();
            if (conn instanceof HttpsURLConnection && !Config.getConfig().isHttpsVerifyPeer()) {
                skipSSL((HttpsURLConnection) conn);
            }
            HttpURLConnection httpUrlConnection = (HttpURLConnection) conn;
            httpUrlConnection.setRequestProperty("Content-Type", "application/json");
            String appId = Config.getConfig().getCloudAppId();
            httpUrlConnection.setRequestProperty("X-OpenRASP-AppID", appId);
            String appSecret = Config.getConfig().getCloudAppSecret();
            httpUrlConnection.setRequestProperty("X-OpenRASP-AppSecret", appSecret);
            httpUrlConnection.setRequestProperty("Accept-Encoding", "gzip");
            httpUrlConnection.setConnectTimeout(DEFAULT_CONNECTION_TIMEOUT);
            httpUrlConnection.setReadTimeout(DEFAULT_READ_TIMEOUT);
            httpUrlConnection.setRequestMethod("POST");
            httpUrlConnection.setUseCaches(false);
            httpUrlConnection.setDoOutput(true);
            httpUrlConnection.setDoInput(true);

            out = new DataOutputStream(httpUrlConnection.getOutputStream());
            out.write(content.getBytes("UTF-8"));
            out.flush();
            httpUrlConnection.connect();
            responseCode = httpUrlConnection.getResponseCode();
            in = httpUrlConnection.getInputStream();
            String encoding = httpUrlConnection.getContentEncoding();
            if (encoding != null && encoding.contains("gzip")) {
                in = new GZIPInputStream(httpUrlConnection.getInputStream());
            }
            jsonString = CloudUtils.convertInputStreamToJsonString(in);
        } finally {
            if (out != null) {
                out.close();
            }
            if (in != null) {
                in.close();
            }
        }
        Gson gson = CloudUtils.getResponseGsonObject();
        GenericResponse response = gson.fromJson(jsonString, new TypeToken<GenericResponse>() {
        }.getType());
        response.setResponseCode(responseCode);
        return response;
    }

    public static void skipSSL(HttpsURLConnection conn) throws NoSuchProviderException, NoSuchAlgorithmException,
            KeyManagementException, MalformedURLException {
        SSLContext sslcontext;
        try {
            sslcontext = SSLContext.getInstance("SSL", "SunJSSE");
        } catch (Exception e) {
            sslcontext = SSLContext.getInstance("SSL");
        }
        sslcontext.init(null, new TrustManager[]{new X509TrustManager() {
            @Override
            public void checkClientTrusted(X509Certificate certificates[], String authType) throws CertificateException {
            }

            @Override
            public void checkServerTrusted(X509Certificate[] ax509certificate, String s) throws CertificateException {
            }

            @Override
            public X509Certificate[] getAcceptedIssuers() {
                return null;
            }
        }}, new java.security.SecureRandom());
        HostnameVerifier ignoreHostnameVerifier = new HostnameVerifier() {
            public boolean verify(String s, SSLSession sslsession) {
                return true;
            }
        };
        conn.setHostnameVerifier(ignoreHostnameVerifier);
        conn.setSSLSocketFactory(sslcontext.getSocketFactory());
    }
}
