/**
 * Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
 * <p>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * <p>
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * <p>
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * <p>
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * <p>
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
public class PluginManager {

    public static final String LOCAL_CHECKER_NAME = "local_checker";
    public static final Logger ALARM_LOGGER = Logger.getLogger(PluginManager.class.getPackage().getName() + ".alarm");
    private static final Logger LOGGER = Logger.getLogger(PluginManager.class.getName());
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

    /**
     * 执行安全检测
     *
     * @param parameter 检测参数 {@link CheckParameter}
     * @return 如果需要对当前请求进行拦截则返回true, 否则返回false
     */
    public static boolean check(CheckParameter parameter) {
        JSContext cx = JSContextFactory.enterAndInitContext();
        return cx.check(parameter);
    }
}
