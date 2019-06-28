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

package com.baidu.openrasp.cloud;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.cloud.model.CloudCacheModel;
import com.baidu.openrasp.cloud.model.CloudRequestUrl;
import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.model.GenericResponse;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.google.gson.Gson;

import java.util.HashMap;
import java.util.Map;
import java.util.TreeMap;

/**
 * @description: 云端统计上报接口
 * @author: anyang
 * @create: 2018/09/28 11:21
 */
public class StatisticsReport extends CloudTimerTask {
    private static final int STATISTICS_REPORT_INTERVAL = 3600;

    public StatisticsReport() {
        super(STATISTICS_REPORT_INTERVAL);
    }

    @Override
    public void execute() {
        TreeMap<Long, Long> temp = new TreeMap<Long, Long>();
        temp.put(System.currentTimeMillis(), HookHandler.requestSum.getAndSet(0));
        if (CloudCacheModel.reportCache.realSize() != 0) {
            for (Map.Entry<Long, Long> entry : CloudCacheModel.reportCache.getEntrySet()) {
                temp.put(entry.getKey(), entry.getValue());
            }
        }
        for (Map.Entry<Long, Long> entry : temp.entrySet()) {
            Map<String, Object> params = new HashMap<String, Object>();
            params.put("rasp_id", CloudCacheModel.getInstance().raspId);
            params.put("time", entry.getKey());
            params.put("request_sum", entry.getValue());
            String content = new Gson().toJson(params);
            String url = CloudRequestUrl.CLOUD_STATISTICS_REPORT_URL;
            GenericResponse response = new CloudHttp().commonRequest(url, content);
            if (response != null) {
                Integer responseCode = response.getResponseCode();
                if (responseCode != null && responseCode >= 200 && responseCode < 300) {
                    CloudCacheModel.reportCache.remove(entry.getKey());
                } else {
                    CloudCacheModel.reportCache.put(entry.getKey(), entry.getValue());
                }
            }
        }
    }

    @Override
    public void handleError(Throwable t) {
        String message = t.getMessage();
        int errorCode = ErrorType.STATISTICSREPORT_ERROR.getCode();
        CloudManager.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), t);
    }

}
