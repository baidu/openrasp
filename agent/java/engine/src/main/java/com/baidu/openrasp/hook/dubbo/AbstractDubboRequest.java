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

package com.baidu.openrasp.hook.dubbo;

import java.util.Map;

/**
 * @author anyang
 * @Description: dubbo的request请求抽象类
 * @date 2018/8/13 15:13
 */
public abstract class AbstractDubboRequest {

    protected static final Class[] EMPTY_CLASS = new Class[]{};

    public Object request;

    public AbstractDubboRequest(Object request) {

        this.request = request;
    }

    public Object getRequest() {
        return request;
    }

    public void setRequest(Object request) {
        this.request = request;
    }


    /**
     * 获取dubbo请求参数的map键值对集合
     * key为参数名称，value为参数值
     *
     * @return 请求参数的map集合
     */
    public abstract Map<String, String[]> getParameterMap();
}
