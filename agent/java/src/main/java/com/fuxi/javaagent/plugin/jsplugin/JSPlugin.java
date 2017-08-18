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

package com.fuxi.javaagent.plugin.jsplugin;

import com.fuxi.javaagent.config.Config;
import com.fuxi.javaagent.plugin.CheckParameter;
import com.fuxi.javaagent.plugin.CheckResult;
import com.fuxi.javaagent.plugin.CheckScript;
import com.fuxi.javaagent.plugin.Plugin;
import org.apache.log4j.Logger;

import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.*;


/**
 * Created by lanyuhang on 4/10/17.
 * All rights reserved
 */

/**
 * JavaScript插件系统
 *
 * 使用线程池复用多个V8实例
 */
public final class JSPlugin extends Plugin {
    private static final Logger LOGGER = Logger.getLogger(JSPlugin.class.getName());

    private int poolSize;
    private long taskTimeout;
    private ExecutorService pool;

    public JSPlugin() {
        this.poolSize = Config.getConfig().getV8ThreadPoolSize();
        this.taskTimeout = Config.getConfig().getV8Timeout();
        this.pool = Executors.newFixedThreadPool(this.poolSize, new JSThreadFactory());
    }

    /**
     * (non-Javadoc)
     *
     * @param scripts 检测插件脚本列表
     * @throws Exception
     */
    public JSPlugin(List<CheckScript> scripts) throws Exception {
        this();
        if (scripts != null) {
            try {
                init(scripts);
            } catch (Exception e) {
                shutdown();
                throw e;
            }
        }
    }

    /**
     * 初始化插件引擎
     *
     * 包括初始化线程池，初始化V8，初始化脚本环境，初始化检测脚本
     *
     * 同步进行，可设置超时时间
     *
     * @param scripts 检测插件脚本列表
     * @throws Exception
     */
    public void init(List<CheckScript> scripts) throws Exception {
        List<JSInitTask> tasks = new LinkedList<JSInitTask>();
        for (int i = 0; i < poolSize; i++) {
            tasks.add(new JSInitTask(scripts));
        }

        List<Future<Long>> futures = pool.invokeAll(tasks, 10000, TimeUnit.MILLISECONDS);
        for (Future<Long> future : futures) {
            if (future.isCancelled()) {
                throw new Exception("initialization timeout");
            }
            future.get();
        }
    }

    /**
     * 执行检测脚本
     *
     * 同步检测，可设置超时时间
     *
     * @param parameter 检测参数集合
     * @return 检测结果列表
     */
    @Override
    public List<CheckResult> check(CheckParameter parameter) {
        JSCheckTask task = new JSCheckTask(parameter);
        List<CheckResult> results = null;
        Future<List<CheckResult>> future = null;
        try {
            future = pool.submit(task);
            results = future.get(taskTimeout, TimeUnit.MILLISECONDS);
        } catch (TimeoutException e) {
            task.terminateExecution();
            future.cancel(true);
            LinkedList<CheckResult> rst = new LinkedList<CheckResult>();
            rst.push(new CheckResult("log", "check timeout", "engine"));
            results = rst;
        } catch (Exception e) {
            LOGGER.error("check failed", e);
        }
        return results;
    }

    /**
     * 关闭线程池
     *
     * 避免内存泄漏
     */
    public void shutdown() {
        if (pool != null) {
            pool.shutdown();
            pool = null;
        }
    }
}
