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

import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.model.GenericResponse;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import org.apache.log4j.helpers.LogLog;

import java.util.concurrent.*;

/**
 * @description: 基于线程池的http请求
 * @author: anyang
 * @create: 2018/09/19 20:12
 */
public class CloudHttpPool extends CloudHttp {
    private final ExecutorService threadPool;

    public CloudHttpPool() {
        int cpuCoreNumber = Runtime.getRuntime().availableProcessors();
        this.threadPool = new ThreadPoolExecutor(2 * cpuCoreNumber, 5 * cpuCoreNumber, 60L, TimeUnit.SECONDS,
                new LinkedBlockingQueue<Runnable>(), new CustomThreadFactory(), new ThreadPoolExecutor.DiscardPolicy());

    }

    @Override
    public GenericResponse logRequest(final String url, final String content) {
        Callable callable = new Callable() {
            @Override
            public GenericResponse call() {
                return CloudHttpPool.super.logRequest(url, content);
            }
        };
        Future<GenericResponse> future = threadPool.submit(callable);
        while (!future.isDone()) {
            try {
                Thread.sleep(10);
            } catch (InterruptedException e) {
                //next loop
            }
        }
        try {
            return future.get();
        } catch (Exception e) {
            String message = "get http result for" + url + "from future failed";
            int errorCode = ErrorType.REQUEST_ERROR.getCode();
            LogLog.warn(CloudUtils.getExceptionObject(message, errorCode).toString(), e);
        }
        return null;
    }

    class CustomThreadFactory implements ThreadFactory {
        @Override
        public Thread newThread(Runnable r) {
            Thread t = Executors.defaultThreadFactory().newThread(r);
            t.setDaemon(true);
            return t;
        }
    }
}