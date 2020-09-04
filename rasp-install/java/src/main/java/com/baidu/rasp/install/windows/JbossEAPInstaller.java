/*
 * Copyright 2017-2020 Baidu Inc.
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

package com.baidu.rasp.install.windows;

import com.baidu.rasp.RaspError;
import com.baidu.rasp.install.BaseStandardInstaller;

import java.util.Scanner;

import static com.baidu.rasp.RaspError.E10001;

/**
 * @description: jbossEAP自动化安装
 * @author: anyang
 * @create: 2019/04/25 18:49
 */
public class JbossEAPInstaller extends BaseStandardInstaller {
    private static final String OPENRASP_START_TAG = "rem BEGIN OPENRASP - DO NOT MODIFY\n";
    private static final String OPENRASP_END_TAG = "rem END OPENRASP\n";
    private static final String OPENRASP_CONFIG = "set \"JAVA_OPTS=%JAVA_OPTS% -javaagent:%JBOSS_HOME%\\rasp\\rasp.jar\"\n";

    public JbossEAPInstaller(String serverName, String serverRoot) {
        super(serverName, serverRoot);
    }

    @Override
    protected String getInstallPath(String serverRoot) {
        return serverRoot + "\\rasp";
    }

    @Override
    protected String getScript(String installPath) {
        return installPath + "\\..\\bin\\standalone.bat";
    }

    @Override
    protected String modifyStartScript(String content) throws RaspError {
        StringBuilder sb = new StringBuilder();
        Scanner scanner = new Scanner(content);
        int modifyConfigState = NOTFOUND;
        boolean isDelete = false;

        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            if (FOUND == modifyConfigState) {
                sb.append(OPENRASP_START_TAG);
                sb.append(OPENRASP_CONFIG);
                sb.append(OPENRASP_END_TAG);
                sb.append(line).append("\n");
                modifyConfigState = DONE;
            }
            // 插入点: rem Setup JBoss specific properties
            if (line.startsWith("rem Setup JBoss specific properties") && modifyConfigState == NOTFOUND) {
                modifyConfigState = FOUND;
            }

            if (line.contains("BEGIN OPENRASP")) {
                isDelete = true;
                continue;
            }
            if (line.contains("END OPENRASP")) {
                isDelete = false;
                continue;
            }
            if (!isDelete) {
                sb.append(line).append("\n");
            }
        }

        if (NOTFOUND == modifyConfigState) {
            throw new RaspError(E10001 + "rem Setup JBoss specific properties");
        }
        return sb.toString();
    }
}
