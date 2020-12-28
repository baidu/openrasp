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
import org.apache.log4j.Logger;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStreamReader;
import java.lang.management.ManagementFactory;
import java.util.ArrayList;

/**
 * @description: rasp 监控java进程的cpu使用情况
 * @author: anyang
 * @create: 2019/06/03 15:38
 */
public class CpuMonitor {
    private static final Logger LOGGER = Logger.getLogger(CpuMonitor.class.getName());
    private static final String CPUS_ALLOWED_LIST = "Cpus_allowed_list";
    private static final String PROCESS_STATUS = "/proc/%d/status";
    private static final int MIN_CPU_DEBUG_LEVEL = 1000;
    private static ArrayList<String> cpuUsageList = new ArrayList<String>(3);

    private boolean isAlive = true;
    private long lastTotalCpuTime;
    private long lastProcessCpuTime;

    CpuMonitor() {
        String pid = getPid();
        this.lastTotalCpuTime = System.currentTimeMillis();
        ProcCpuProcess processcpu = new ProcCpuProcess(pid);
        this.lastProcessCpuTime = processcpu.getProcessTotalCpuTime();
    }

    private float getCpuUsage() {
        float totalUsage = 0;
        try {
            String pid = getPid();
            long currentTotalCpuTime = System.currentTimeMillis();
            ProcCpuProcess cupProcess = new ProcCpuProcess(pid);
            long currentProcessCpuTime = cupProcess.getProcessTotalCpuTime();
            totalUsage = (float) (currentProcessCpuTime - this.lastProcessCpuTime) * 10 * 100 / (float) (currentTotalCpuTime - this.lastTotalCpuTime);
            this.lastTotalCpuTime = currentTotalCpuTime;
            this.lastProcessCpuTime = currentProcessCpuTime;
        } catch (Exception e) {
            LogTool.warn(ErrorType.CPU_ERROR, "count cpu usage failed: " + e.getMessage(), e);
        }
        return totalUsage;
    }

    private String getPid() {
        String name = ManagementFactory.getRuntimeMXBean().getName();
        return name.split("@")[0];
    }

    private int getCpuUsageNumber(String pid) {
        int totalCpuNum = 0;
        try {
            String path = PROCESS_STATUS.replace("%d", pid);
            BufferedReader reader = new BufferedReader(new InputStreamReader(new FileInputStream(new File(path))));
            String line = null;
            while ((line = reader.readLine()) != null) {
                if (line.startsWith(CPUS_ALLOWED_LIST)) {
                    line = line.trim();
                    String[] temp = line.split("\\s+");
                    if (temp.length >= 2) {
                        if (Config.getConfig().getDebugLevel() > MIN_CPU_DEBUG_LEVEL) {
                            LOGGER.info(line);
                        }
                        for (String s : temp[1].split(",")) {
                            if (s.contains("-")) {
                                String[] num = s.split("-");
                                totalCpuNum += (Integer.parseInt(num[1]) - Integer.parseInt(num[0]) + 1);
                            } else {
                                totalCpuNum++;
                            }
                        }
                    }
                    break;
                }
            }
        } catch (Exception e) {
            LogTool.warn(ErrorType.CPU_ERROR, "get server occupied cpu number failed: " + e.getMessage(), e);
        }
        return totalCpuNum;
    }

    private void checkCpuUsage() {
        float totalCpuUsage = getCpuUsage();
        int cpuUsageNum = getCpuUsageNumber(getPid());
        float cpuUsageUpper = cpuUsageNum * Config.getConfig().getCpuUsagePercent();
        if (Config.getConfig().getDebugLevel() > MIN_CPU_DEBUG_LEVEL) {
            LOGGER.info("current cpu usage: " + totalCpuUsage);
        }
        if (totalCpuUsage > cpuUsageUpper) {
            if (!Config.getConfig().getDisableHooks()) {
                cpuUsageList.add(totalCpuUsage + "%");
                if (cpuUsageList.size() >= 3) {
                    Config.getConfig().setDisableHooks("true");
                    LOGGER.info("the last three time cpu usages are " + cpuUsageList.toString() + " with "
                            + cpuUsageNum + " cores, " + "already disabled the rasp");
                }
            }
        } else {
            Config.getConfig().setDisableHooks("false");
            if (cpuUsageList.size() >= 3) {
                LOGGER.info("the last cpu usage is " + totalCpuUsage + " with "
                        + cpuUsageNum + " cores, " + "already enable the rasp again");
            }
            cpuUsageList.clear();
        }
    }

    public void start() {
        Thread thread = new Thread(new CpuMonitorThread());
        thread.setDaemon(true);
        thread.start();
    }

    public void stop() {
        this.isAlive = false;
    }

    class CpuMonitorThread implements Runnable {
        @Override
        public void run() {
            while (isAlive) {
                try {
                    // 如果关闭了 cpu 监控功能，开启 hook 点开关，清除 cpu 监控缓存，并挂起该线程
                    synchronized (CpuMonitorManager.cpuMonitor) {
                        while (!Config.getConfig().getCpuUsageEnable()) {
                            cpuUsageList.clear();
                            Config.getConfig().setDisableHooks("false");
                            CpuMonitorManager.cpuMonitor.wait();
                        }
                    }
                    Thread.sleep(Config.getConfig().getCpuUsageCheckInterval() * 1000);
                    checkCpuUsage();
                } catch (Throwable e) {
                    LogTool.warn(ErrorType.CPU_ERROR, e.getMessage(), e);
                }
            }
        }
    }

}
