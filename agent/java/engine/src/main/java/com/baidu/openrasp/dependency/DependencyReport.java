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

package com.baidu.openrasp.dependency;

import com.baidu.openrasp.cloud.CloudHttp;
import com.baidu.openrasp.cloud.CloudTimerTask;
import com.baidu.openrasp.cloud.model.CloudCacheModel;
import com.baidu.openrasp.cloud.model.CloudRequestUrl;
import com.baidu.openrasp.cloud.model.GenericResponse;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.messaging.ErrorType;
import com.baidu.openrasp.messaging.LogTool;
import com.google.gson.Gson;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.logging.Logger;

/**
 * @description: 依赖检查上报
 * @author: anyang
 * @create: 2019/04/19 16:07
 */
public class DependencyReport extends CloudTimerTask {

    public static final Logger LOGGER = Logger.getLogger(DependencyReport.class.getName());
    private static int reportTimes = 0;
    private static final int FIRST_INTERNAL = 15;
    private static final int INIT_INTERNAL = 120;

    public DependencyReport() {
        super("OpenRASP Dependency Report Thread");
    }

    @Override
    public long getSleepTime() {
        if (reportTimes < 3) {
            reportTimes++;
            return FIRST_INTERNAL;
        } else if (reportTimes < 6) {
            reportTimes++;
            return INIT_INTERNAL;
        }
        return Config.getConfig().getDependencyCheckInterval();
    }

    @Override
    public void execute() {
        HashSet<Dependency> dependencyHashSet = DependencyFinder.getDependencySet();
        Map<String, Object> parameters = new HashMap<String, Object>();
        parameters.put("rasp_id", CloudCacheModel.getInstance().getRaspId());
        parameters.put("dependency", dependencyHashSet);
        LOGGER.info("start reporting " + dependencyHashSet.size() + " dependencies");
        String url = CloudRequestUrl.CLOUD_DEPENDENCY_REPORT_URL;
        GenericResponse response = new CloudHttp().commonRequest(url, new Gson().toJson(parameters));
        if (!CloudUtils.checkResponse(response)) {
            LogTool.warn(ErrorType.DEPENDENCY_REPORT_ERROR,
                    CloudUtils.handleError(ErrorType.DEPENDENCY_REPORT_ERROR, response));
        }
    }

    @Override
    public void handleError(Throwable t) {
        LogTool.warn(ErrorType.REGISTER_ERROR, "dependency report failed: " + t.getMessage(), t);
    }
}
