/*
 * Copyright 2017-2021 Baidu Inc.
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

package com.baidu.openrasp.tool.filemonitor;

import com.baidu.openrasp.config.Config;
import com.fuxi.javaagent.contentobjects.jnotify.JNotify;
import com.fuxi.javaagent.contentobjects.jnotify.JNotifyException;
import org.apache.commons.io.monitor.FileAlterationListener;
import org.apache.commons.io.monitor.FileAlterationMonitor;
import org.apache.commons.io.monitor.FileAlterationObserver;

import java.io.File;

/**
 * Created by tyy on 4/17/17.
 * 文件事件的监视器，监视单位为文件夹
 */
public class FileScanMonitor {

    static {
        if (!Config.getConfig().getCloudSwitch() && Config.getConfig().getFileMonitorMode().equals("jnotify")) {
            JnotifyWatcher watcher = new JnotifyWatcher();
            JNotify.init(Config.baseDirectory, watcher);
        }
    }

    public FileAlterationMonitor monitor;

    /**
     * 增加监视器
     *
     * @param path     监听的文件夹路径
     * @param listener 事件回调接口
     * @return 监听器的id
     * @throws JNotifyException {@link JNotifyException}
     */
    public static Object addMonitor(String path, FileAlterationListener listener) throws Exception {
        if (!Config.getConfig().getCloudSwitch()) {
            if ("jnotify".equals(Config.getConfig().getFileMonitorMode())) {
                File file = new File(path);
                FileAlterationObserver observer = new FileAlterationObserver(file);
                observer.checkAndNotify();
                observer.addListener(listener);
                int mask = JNotify.FILE_CREATED | JNotify.FILE_DELETED
                        | JNotify.FILE_MODIFIED;
                return JNotify.addWatch(path, mask, false, new FileEventListener(observer));
            } else if ("scan".equals(Config.getConfig().getFileMonitorMode())) {
                FileAlterationObserver observer = new FileAlterationObserver(path);
                observer.addListener(listener);
                FileAlterationMonitor monitor = new FileAlterationMonitor(Config.getConfig().getFileMonitorInterval() * 1000, observer);
                monitor.start();
                return monitor;
            }
        }

        return 0;
    }

    /**
     * 移除某一个文件夹监听
     *
     * @param watchId 增加文件夹监听的时候返回的监听器id
     */
    public static void removeMonitor(Object watchId) {
        if (!Config.getConfig().getCloudSwitch()) {
            if ("jnotify".equals(Config.getConfig().getFileMonitorMode())
                    && (watchId instanceof Integer)) {
                try {
                    JNotify.removeWatch((Integer) watchId);
                } catch (JNotifyException e) {
                    e.printStackTrace();
                }
            } else if ("scan".equals(Config.getConfig().getFileMonitorMode())
                    && watchId instanceof FileAlterationMonitor) {
                try {
                    ((FileAlterationMonitor) watchId).stop();
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }


}
