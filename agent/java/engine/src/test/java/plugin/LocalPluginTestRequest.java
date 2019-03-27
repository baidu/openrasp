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

package plugin;

import java.util.*;

import com.baidu.openrasp.request.AbstractRequest;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;

public class LocalPluginTestRequest extends AbstractRequest {

    private JsonObject context;
    private JsonObject parameter;

    public LocalPluginTestRequest(JsonObject requestInfoJson) {
        super(0);
        context = (JsonObject) requestInfoJson.get("context");
        parameter = (JsonObject) context.get("parameter");
    }

    /**
     * 获取本服务器地址
     *
     * @return 服务器地址
     */
    @Override
    public String getLocalAddr() {
        if (context.has("localAddr")) {
            return context.get("localAddr").getAsString();
        }
        return null;
    }

    /**
     * 获取请求方法
     *
     * @return 请求方法
     */
    @Override
    public String getMethod() {
        if (context.has("method")) {
            return context.get("method").getAsString();
        }
        return null;
    }

    /**
     * 获取请求协议
     *
     * @return 请求协议
     */
    @Override
    public String getProtocol() {
        if (context.has("protocol")) {
            return context.get("protocol").getAsString();
        }
        return null;
    }

    /**
     * 获取验证类型
     *
     * @return 验证类型
     */
    @Override
    public String getAuthType() {
        if (context.has("authType")) {
            return context.get("authType").getAsString();
        }
        return null;
    }

    /**
     * 获取请求路径
     *
     * @return 请求路径
     */
    @Override
    public String getContextPath() {
        if (context.has("contextPath")) {
            return context.get("contextPath").getAsString();
        }
        return null;
    }

    /**
     * 获取访问客户端的地址
     *
     * @return 客户端地址
     */
    @Override
    public String getRemoteAddr() {
        if (context.has("remoteAddr")) {
            return context.get("remoteAddr").getAsString();
        }
        return null;
    }

    /**
     * 获取请求的uri
     *
     * @return 请求uri
     */
    @Override
    public String getRequestURI() {
        if (context.has("requestUri")) {
            return context.get("requestUri").getAsString();
        }
        return null;
    }

    /**
     * 获取请求的url
     *
     * @return 请求的url
     */
    @Override
    public StringBuffer getRequestURL() {
        if (context.has("requestUrl")) {
            return new StringBuffer(context.get("requestUrl").getAsString());
        }
        return null;
    }

    /**
     * 获取服务器名称
     *
     * @return 服务器名称
     */
    @Override
    public String getServerName() {
        if (context.has("serverName")) {
            return context.get("serverName").getAsString();
        }
        return null;
    }

    /**
     * 根据请求的参数名称，获取请求参数的值
     *
     * @param key 请求参数名称
     * @return 请求参数的值
     */
    @Override
    public String getParameter(String key) {
        if (parameter.has(key)) {
            return parameter.get(key).getAsJsonArray().get(0).getAsString();
        }
        return null;
    }

    /**
     * 获取所有请求参数名称
     *
     * @return 请求参数名称的枚举集合
     */
    @Override
    public Enumeration<String> getParameterNames() {
        Vector<String> paramNames = new Vector<String>();
        for (Map.Entry<String, JsonElement> stringJsonElementEntry : parameter.entrySet()) {
            paramNames.add(stringJsonElementEntry.getKey());
        }
        return paramNames.elements();
    }

    /**
     * 获取请求参数的map键值对集合
     * key为参数名称，value为参数值
     *
     * @return 请求参数的map集合
     */
    @Override
    public Map<String, String[]> getParameterMap() {
        Iterator<Map.Entry<String, JsonElement>> entrySetIterator = parameter.entrySet().iterator();
        Map<String, String[]> result = new HashMap<String, String[]>();
        Map.Entry<String, JsonElement> paraEntry;
        while (entrySetIterator.hasNext()) {
            paraEntry = entrySetIterator.next();
            List<String> value = new ArrayList<String>();
            for (JsonElement jsonElement : paraEntry.getValue().getAsJsonArray()) {
                value.add(jsonElement.getAsString());
            }
            final int size = value.size();
            String[] valueArray = value.toArray(new String[size]);
            result.put(paraEntry.getKey(), valueArray);
        }
        return result;
    }

    /**
     * 根据请求头的名称获取请求头的值
     *
     * @param key 请求头的名称
     * @return 请求头的值
     */
    @Override
    public String getHeader(String key) {
        if (context.has("header")) {
            JsonObject headers = context.get("header").getAsJsonObject();
            if (headers.has(key)) {
                return headers.get(key).getAsString();
            }
        }
        return null;
    }

    /**
     * 获取所有请求头的名称
     *
     * @return 请求头名称的枚举集合
     */
    @Override
    public Enumeration<String> getHeaderNames() {
        if (context.has("header")) {
            Vector<String> headerNames = new Vector<String>();
            for (Map.Entry<String, JsonElement> stringJsonElementEntry : context.get("header").getAsJsonObject().entrySet()) {
                headerNames.add(stringJsonElementEntry.getKey());
            }
            return headerNames.elements();
        }
        return null;
    }

    /**
     * 获取请求的url中的 Query String 参数部分
     *
     * @return 请求的 Query String
     */
    @Override
    public String getQueryString() {
        if (context.has("querystring")) {
            return context.get("querystring").getAsString();
        }
        return null;
    }

    /**
     * 获取服务器的上下文参数map集合
     * key为参数名字，value为参数的值
     *
     * @return 服务器上下文参数的map集合
     */
    @Override
    public Map<String, String> getServerContext() {
        return null;
    }

    /**
     * 获取app部署根路径
     *
     * @return app部署根路径
     */
    @Override
    public String getAppBasePath() {
        if (context.has("appbasepath")) {
            return context.get("appbasepath").getAsString();
        }
        return null;
    }

    /**
     * 获取自定义的clientip
     *
     * @return 自定义的clientip
     */
    @Override
    public String getClinetIp() {
        return null;
    }

    /**
     * 获取请求的contentType
     *
     * @return contentType
     */
    @Override
    public String getContentType() {
        return null;
    }

    /**
     * 返回body的编码类型
     *
     * @return CharacterEncoding
     */
    public String getCharacterEncoding(){
        return null;
    }
}
