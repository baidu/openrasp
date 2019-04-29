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

package com.baidu.openrasp.dependency;

import com.baidu.openrasp.cloud.CloudHttp;
import com.baidu.openrasp.cloud.CloudManager;
import com.baidu.openrasp.cloud.model.CloudCacheModel;
import com.baidu.openrasp.cloud.model.CloudRequestUrl;
import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.model.GenericResponse;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.google.gson.Gson;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;

/**
 * @description: 依赖检查上报
 * @author: anyang
 * @create: 2019/04/19 16:07
 */
public class DependencyReport {
    private static final int REGISTER_DELAY = 60 * 1000;

    public DependencyReport() {
        Thread thread = new Thread(new DependencyThread());
        thread.setDaemon(true);
        thread.start();
    }

    class DependencyThread implements Runnable {
        @Override
        public void run() {
            while (true) {
                try {
                    HashSet<Dependency> dependencyHashSet = DependencyFinder.getDependencySet();
                    Map<String, Object> parameters = new HashMap<String, Object>();
                    parameters.put("rasp_id", CloudCacheModel.getInstance().getRaspId());
                    parameters.put("dependency", dependencyHashSet);
                    String url = CloudRequestUrl.CLOUD_DEPENDENCY_REPORT_URL;
                    GenericResponse response = new CloudHttp().commonRequest(url, new Gson().toJson(parameters));
                    if (!CloudUtils.checkRequestResult(response)) {
                        String message = CloudUtils.handleError(ErrorType.DEPENDENCY_REPORT_ERROR, response);
                        int errorCode = ErrorType.REGISTER_ERROR.getCode();
                        CloudManager.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode));
                    }
                    Thread.sleep(REGISTER_DELAY);
                } catch (Exception e) {
                    String message = e.getMessage();
                    int errorCode = ErrorType.REGISTER_ERROR.getCode();
                    CloudManager.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode));
                }
            }
        }
    }
}
