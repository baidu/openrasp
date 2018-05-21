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

import java.util.concurrent.ExecutorService;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

/**
 * Created by lxk on 9/12/17.
 */
public class AsyncHttpClient extends HttpClient {
    private final ExecutorService threadPool;
    private final int cpuCoreSize;

    public AsyncHttpClient() {
        super();
        cpuCoreSize = Runtime.getRuntime().availableProcessors();
        threadPool = new ThreadPoolExecutor(cpuCoreSize + 1, 5 * cpuCoreSize +1, 60L, TimeUnit.SECONDS,
                new LinkedBlockingQueue<Runnable>(), new ThreadPoolExecutor.DiscardPolicy());
    }

    @Override
    public void request(final String requestUrl, final String attackInfoJson, final int connectionTimeout, final int readTimeout) {
        threadPool.execute(new Runnable() {
            @Override
            public void run() {
                AsyncHttpClient.super.request(requestUrl, attackInfoJson, connectionTimeout, readTimeout);
            }
        });
    }

    /**
     * 判断当阻塞队列size小于cpucore数目
     * @return
     */
    public boolean shouldSend() {
        if (threadPool instanceof ThreadPoolExecutor) {
            return ((ThreadPoolExecutor) threadPool).getQueue().size() < cpuCoreSize;
        }
        return true;
    }
}
