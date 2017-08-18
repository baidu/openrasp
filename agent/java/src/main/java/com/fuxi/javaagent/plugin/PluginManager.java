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

package com.fuxi.javaagent.plugin;

import com.fuxi.javaagent.HookHandler;
import com.fuxi.javaagent.config.Config;
import com.fuxi.javaagent.plugin.jsplugin.JSPlugin;
import com.fuxi.javaagent.tool.filemonitor.FileScanListener;
import com.fuxi.javaagent.tool.filemonitor.FileScanMonitor;
import org.apache.commons.io.filefilter.FileFilterUtils;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.FileFilter;
import java.util.LinkedList;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;


/**
 * Created by tyy on 4/5/17.
 * All rights reserved
 */

/**
 * PluginManager是一个静态类，封装了插件系统的细节，仅对外层暴露init和check方法
 *
 * PluginManager内部管理插件系统实例，监控检测脚本文件变化
 *
 * 必须首先初始化
 */
public class PluginManager {
    private static final Logger LOGGER = Logger.getLogger(PluginManager.class.getName());
    private static final Logger ALARM_LOGGER = Logger.getLogger(PluginManager.class.getPackage().getName() + ".alarm");

    private static JSPlugin plugin = null;
    private static Timer timer = null;
    private static Integer watchId = null;
    private static ReadWriteLock lock = new ReentrantReadWriteLock();

    /**
     * 整体初始化检测脚本文件监控和插件引擎
     *
     * @throws Exception
     * @see #initFileWatcher()
     * @see #initPlugin()
     */
    public synchronized static void init() throws Exception {
        initPlugin();
        initFileWatcher();
    }

    /**
     * 初始化检测脚本文件监控
     *
     * 不调用则不会在运行时自动更新检测脚本
     *
     * @throws Exception
     */
    public synchronized static void initFileWatcher() throws Exception {
        boolean oldValue = HookHandler.enableHook.getAndSet(false);
        if (watchId != null) {
            FileScanMonitor.removeMonitor(watchId);
            watchId = null;
        }
        watchId = FileScanMonitor.addMonitor(
                Config.getConfig().getScriptDirectory(),
                new FileScanListener() {
                    @Override
                    public void onFileCreate(File file) {
                        updatePluginAsync();
                    }

                    @Override
                    public void onFileChange(File file) {
                        updatePluginAsync();
                    }

                    @Override
                    public void onFileDelete(File file) {
                        updatePluginAsync();
                    }
                });
        HookHandler.enableHook.set(oldValue);
    }

    /**
     * 初始化插件引擎
     *
     * @throws Exception
     */
    public synchronized static void initPlugin() throws Exception {
        updatePlugin();
    }

    /**
     * 更新插件引擎
     *
     * 检测脚本变化时更新
     *
     * 当新插件引擎初始化成功之后再替换旧插件引擎
     *
     * @throws Exception
     */
    private synchronized static void updatePlugin() throws Exception {
        boolean oldValue = HookHandler.enableHook.getAndSet(false);
        File pluginDir = new File(Config.getConfig().getScriptDirectory());
        LOGGER.debug("plugin directory: " + pluginDir.getAbsolutePath());
        if (!pluginDir.isDirectory()) {
            pluginDir.mkdir();
        }
        File[] pluginFiles = pluginDir.listFiles((FileFilter) FileFilterUtils.suffixFileFilter(".js"));
        List<CheckScript> scripts = new LinkedList<CheckScript>();
        for (File file : pluginFiles) {
            try {
                scripts.add(new CheckScript(file));
            } catch (Exception e) {
                LOGGER.error("", e);
            }
        }
        JSPlugin newJSPlugin = new JSPlugin(scripts);
        JSPlugin oldJSPlugin = plugin;

        lock.writeLock().lock();
        plugin = newJSPlugin;
        lock.writeLock().unlock();

        if (oldJSPlugin != null) {
            oldJSPlugin.shutdown();
            oldJSPlugin = null;
        }
        HookHandler.enableHook.set(oldValue);
    }

    /**
     * 异步更新插件引擎
     *
     * 可避免文件系统中脚本文件更新时产生的抖动
     *
     * 若产生抖动，可适量增大定时器延时
     */
    private synchronized static void updatePluginAsync() {
        if (timer != null) {
            timer.cancel();
            timer = null;
        }
        timer = new Timer();
        timer.schedule(new TimerTask() {
            @Override
            public void run() {
                try {
                    updatePlugin();
                } catch (Exception e) {
                    LOGGER.error("", e);
                }
                if (timer != null) {
                    timer.cancel();
                    timer = null;
                }
            }
        }, 500);
    }

    /**
     * 执行安全检测
     *
     * @param parameter 检测参数 {@link CheckParameter}
     * @return 如果需要对当前请求进行拦截则返回true, 否则返回false
     */
    public static boolean check(CheckParameter parameter) {
        boolean block = false;

        List<CheckResult> results;

        lock.readLock().lock();
        if (plugin == null) {
            return block;
        }
        results = plugin.check(parameter);
        lock.readLock().unlock();

        if (results != null && parameter != null) {
            if (LOGGER.isDebugEnabled()) {
                LOGGER.debug(parameter + ", " + results);
            }

            for (CheckResult result : results) {
                if (result.getResult().equals("block")) {
                    block = true;
                }

                // 输出报警日志
                if (!result.getResult().equals("ignore")) {
                    AttackInfo attackInfo = new AttackInfo(parameter, result);
                    ALARM_LOGGER.info(attackInfo.toString());
                }
            }
        }
        return block;
    }
}
