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

package com.baidu.openrasp.plugin.js.engine;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.tool.filemonitor.FileScanListener;
import com.baidu.openrasp.tool.filemonitor.FileScanMonitor;
import org.apache.commons.io.filefilter.FileFilterUtils;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.FileFilter;
import java.util.LinkedList;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;
/**
 * Created by tyy on 4/5/17.
 * All rights reserved
 */

/**
 * PluginManager是一个静态类，封装了插件系统的细节，仅对外层暴露init和check方法
 * <p>
 * PluginManager内部管理插件系统实例，监控检测脚本文件变化
 * <p>
 * 必须首先初始化
 */
public class JsPluginManager {

    private static final Logger LOGGER = Logger.getLogger(JsPluginManager.class.getName());
    private static Timer timer = null;
    private static Integer watchId = null;

    /**
     * 初始化插件引擎
     *
     * @throws Exception
     */
    public synchronized static void init() throws Exception {
        JSContextFactory.init();
        updatePlugin();
        initFileWatcher();
    }

    /**
     * 初始化检测脚本文件监控
     * <p>
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
                        if (file.getName().endsWith(".js")) {
                            updatePluginAsync();
                        }
                    }

                    @Override
                    public void onFileChange(File file) {
                        if (file.getName().endsWith(".js")) {
                            updatePluginAsync();
                        }
                    }

                    @Override
                    public void onFileDelete(File file) {
                        if (file.getName().endsWith(".js")) {
                            updatePluginAsync();
                        }
                    }
                });
        HookHandler.enableHook.set(oldValue);
    }

    /**
     * 更新插件引擎
     * <p>
     * 检测脚本变化时更新
     * <p>
     * 当新插件引擎初始化成功之后再替换旧插件引擎
     *
     * @throws Exception
     */
    private synchronized static void updatePlugin() throws Exception {
        // 清空 algorithm.config 配置
        Config.getConfig().setAlgorithmConfig("{}");
        boolean oldValue = HookHandler.enableHook.getAndSet(false);
        File pluginDir = new File(Config.getConfig().getScriptDirectory());
        LOGGER.debug("checker directory: " + pluginDir.getAbsolutePath());
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

        JSContextFactory.setCheckScriptList(scripts);

        HookHandler.enableHook.set(oldValue);
    }

    /**
     * 异步更新插件引擎
     * <p>
     * 可避免文件系统中脚本文件更新时产生的抖动
     * <p>
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


}
