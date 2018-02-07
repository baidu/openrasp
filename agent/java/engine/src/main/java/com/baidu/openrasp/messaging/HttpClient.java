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

package com.baidu.openrasp.messaging;

import java.io.DataOutputStream;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;

/**
 * Created by lxk on 9/12/17.
 */
public class HttpClient {

    public static final String CONTENT_TYPE = "Content-Type";
    public static final String CONNECTION = "Connection";
    public static final String KEEP_ALIVE = "Keep-Alive";
    public static final String CONTENT_LENGTH = "Content-Length";
    public static final String MEDIA_TYPE = "application/json; charset=UTF-8";
    public static final String REQUEST_METHOD = "POST";


    /**
     * 用于post推送報警的http client
     *
     */
    public HttpClient() {}

    protected void request(String requestUrl, String attackInfoJson, int connectionTimeout, int readTimeout) {
        HttpURLConnection httpURLConnection = null;
        try {
            URL url = new URL(requestUrl);
            httpURLConnection = (HttpURLConnection) url.openConnection();
            httpURLConnection.setConnectTimeout(connectionTimeout);
            httpURLConnection.setReadTimeout(readTimeout);
            httpURLConnection.setUseCaches(false);
            httpURLConnection.setRequestMethod(REQUEST_METHOD);
            httpURLConnection.setDoInput(true);
            httpURLConnection.setDoOutput(true);
            byte[] content = attackInfoJson.getBytes();
            httpURLConnection.setRequestProperty(CONTENT_TYPE, MEDIA_TYPE);
            httpURLConnection.setRequestProperty(CONNECTION, KEEP_ALIVE);
            httpURLConnection.setRequestProperty(CONTENT_LENGTH, Long.toString(content.length));
            httpURLConnection.setFixedLengthStreamingMode(content.length);
            DataOutputStream wr = new DataOutputStream(httpURLConnection.getOutputStream());
            wr.write(content);
            wr.flush();
            wr.close();
            int responseCode = httpURLConnection.getResponseCode();
            if (responseCode >= 300) {
                System.out.println("[OpenRASP] " + responseCode + "return while posting attack info to " + requestUrl);
            }
        } catch (MalformedURLException me) {
            System.out.println("[OpenRASP] " + me.getMessage());
            me.printStackTrace();
        } catch (Exception e) {
            System.out.println("[OpenRASP] " + e.getMessage());
            e.printStackTrace();
        } finally {
            if (httpURLConnection != null) {
                httpURLConnection.disconnect();
            }
        }
    }


}
