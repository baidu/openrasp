package com.baidu.openrasp.cloud;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.cloud.model.CloudCacheModel;
import com.baidu.openrasp.cloud.model.CloudRequestUrl;
import com.baidu.openrasp.cloud.model.GenericResponse;
import com.baidu.openrasp.config.Config;
import com.google.gson.Gson;

import java.util.HashMap;
import java.util.Map;
import java.util.TreeMap;

/**
 * @program openrasp
 * @description: 云端统计上报接口
 * @author: anyang
 * @create: 2018/09/28 11:21
 */
public class StatisticsReport {
    private static final int STATISTICS_REPORT_INTERVAL = 30 * 1000;

    public StatisticsReport() {
        new Thread(new StatisticsReportThread()).start();
    }

    class StatisticsReportThread implements Runnable {
        @Override
        public void run() {
            while (true) {
                TreeMap<Long, Long> temp = new TreeMap<Long, Long>();
                temp.put(System.nanoTime(), HookHandler.TOTAL_REQUEST_NUM.longValue());
                if (!CloudCacheModel.reportCache.isEmpty()) {
                    temp.putAll(CloudCacheModel.reportCache);
                }
                for (Map.Entry<Long, Long> entry : temp.entrySet()) {
                    Map<String, Object> params = new HashMap<String, Object>();
                    params.put("rasp_id", CloudCacheModel.getInstance().raspId);
                    params.put("time", entry.getKey());
                    params.put("request_sum", entry.getValue());
                    String content = new Gson().toJson(params);
                    String url = CloudRequestUrl.CLOUD_STATISTICS_REPORT_URL;
                    GenericResponse response = new CloudHttp().request(url, content);
                    if (response == null) {
                        CloudCacheModel.reportCache.put(entry.getKey(), entry.getValue());
                    } else {
                        Integer responseCode = response.getResponseCode();
                        if (responseCode != null && responseCode >= 200 && responseCode < 300) {
                            CloudCacheModel.reportCache.remove(entry.getKey());
                        } else {
                            CloudCacheModel.reportCache.put(entry.getKey(), entry.getValue());
                        }
                    }
                    try {
                        Thread.sleep(STATISTICS_REPORT_INTERVAL);
                    } catch (InterruptedException e) {
                        CloudCacheModel.reportCache.put(entry.getKey(), entry.getValue());
                    }

                }

            }
        }
    }
}
