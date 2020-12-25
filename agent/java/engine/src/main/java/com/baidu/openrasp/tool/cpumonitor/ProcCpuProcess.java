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

import com.baidu.openrasp.messaging.ErrorType;
import com.baidu.openrasp.messaging.LogTool;
import org.apache.commons.io.FileUtils;

import java.io.File;

/**
 * @description: 统计特定进程的cpu利用率
 * @author: anyang
 * @create: 2019/06/05 11:20
 */
public class ProcCpuProcess {
    public static final String PROCESS_STAT = "/proc/%d/stat";
    private Long utime;
    private Long stime;
    private Long cutime;
    private Long cstime;

    public ProcCpuProcess(String pid) {
        String path = PROCESS_STAT.replace("%d", pid);
        File file = new File(path);
        if (file.exists() && file.isFile() && file.canRead()) {
            try {
                String content = FileUtils.readFileToString(file);
                String[] fields = content.trim().split("\\s+");
                this.utime = Long.parseLong(fields[13]);
                this.stime = Long.parseLong(fields[14]);
                this.cutime = Long.parseLong(fields[15]);
                this.cstime = Long.parseLong(fields[16]);
            } catch (Exception e) {
                LogTool.warn(ErrorType.CPU_ERROR, "get server process cpu usage failed: " + e.getMessage(), e);
            }
        }
    }

    public long getProcessTotalCpuTime() {
        if (utime != null && stime != null && cutime != null && cstime != null) {
            return utime + stime + cutime + cstime;
        }
        return 0;
    }
}
