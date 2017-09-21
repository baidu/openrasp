/**
 * Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

package com.fuxi.javaagent.messaging;

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
                System.out.println(responseCode + "return while posting attack info to " + requestUrl);
            }
        } catch (MalformedURLException me) {
            System.out.println(me.getMessage());
            me.printStackTrace();
        } catch (Exception e) {
            System.out.println(e.getMessage());
            e.printStackTrace();
        } finally {
            if (httpURLConnection != null) {
                httpURLConnection.disconnect();
            }
        }
    }


}
