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

package com.baidu.openrasp.tool.cpumonitor;

import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.messaging.ErrorType;
import com.baidu.openrasp.messaging.LogTool;
import com.baidu.openrasp.tool.OSUtil;

/**
 * @description: cpu监控管理类
 * @author: anyang
 * @create: 2019/06/10 20:07
 */
public class CpuMonitorManager {
    static final CpuMonitor cpuMonitor = new CpuMonitor();

    public static void start() {
        if (Config.getConfig().getCpuUsageEnable()) {
            if (OSUtil.isLinux()) {
                cpuMonitor.start();
            } else {
                LogTool.warn(ErrorType.CPU_ERROR, "only support the cpu monitor in linux OS");
            }
        }
    }

    public static void resume(boolean isEnable) {
        if (OSUtil.isLinux()) {
            synchronized (cpuMonitor) {
                if (isEnable) {
                    cpuMonitor.notify();
                }
            }
        }
    }

    public static void release() {
        if (Config.getConfig().getCpuUsageEnable() && OSUtil.isLinux()) {
            cpuMonitor.stop();
        }
    }
}
