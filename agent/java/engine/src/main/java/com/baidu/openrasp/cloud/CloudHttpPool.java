package com.baidu.openrasp.cloud;

import java.util.concurrent.*;

/**
 * @description: 基于线程池的http请求
 * @author: anyang
 * @create: 2018/09/19 20:12
 */
public class CloudHttpPool extends CloudHttp {
    private final ExecutorService threadPool;
    private final int cpuCoreNumber;

    public CloudHttpPool() {
        this.cpuCoreNumber = Runtime.getRuntime().availableProcessors();
        this.threadPool = new ThreadPoolExecutor(2 * cpuCoreNumber, 5 * cpuCoreNumber, 60L, TimeUnit.SECONDS,
                new LinkedBlockingQueue<Runnable>(), new ThreadPoolExecutor.DiscardPolicy());

    }

    @Override
    public String request(final String url, final String content) {
        Callable callable = new Callable() {
            @Override
            public String call() {
                return CloudHttpPool.super.request(url, content);
            }
        };
        Future<String> future = threadPool.submit(callable);
        while (!future.isDone()) {
            try {
                Thread.sleep(10);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        try {
            return future.get();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }
}
