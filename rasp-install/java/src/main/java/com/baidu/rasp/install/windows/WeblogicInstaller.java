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

package com.baidu.rasp.install.windows;

import com.baidu.rasp.RaspError;
import com.baidu.rasp.install.BaseStandardInstaller;

import java.util.Scanner;
import java.util.regex.Pattern;

import static com.baidu.rasp.RaspError.E10001;

/**
 * @description: weblogic安装
 * @author: anyang
 * @create: 2018/09/06 16:01
 */
public class WeblogicInstaller extends BaseStandardInstaller {

    private static String OPENRASP_CONFIG =
            "rem BEGIN OPENRASP - DO NOT MODIFY" + LINE_SEP +
                    "set JAVA_OPTIONS=\"-javaagent:%DOMAIN_HOME%/rasp/rasp.jar %JAVA_OPTIONS%\"" + LINE_SEP +
                    "rem END OPENRASP" + LINE_SEP;
    private static Pattern OPENRASP_REGEX = Pattern.compile(".*(\\s*OPENRASP\\s*|JAVA_OPTIONS.*/rasp/).*");

    public WeblogicInstaller(String serverName, String serverRoot) {
        super(serverName, serverRoot);
    }

    @Override
    protected String getInstallPath(String serverRoot) {
        return serverRoot + "/rasp";
    }

    @Override
    protected String getScript(String installPath) {
        return installPath + "/../bin/startWebLogic.cmd";
    }

    @Override
    protected String modifyStartScript(String content) throws RaspError {
        int modifyConfigState = NOTFOUND;
        StringBuilder sb = new StringBuilder();
        Scanner scanner = new Scanner(content);
        while (scanner.hasNext()) {
            String line = scanner.nextLine();
            if (line.startsWith("set JAVA_OPTIONS") && line.contains("set JAVA_OPTIONS=\"${SAVE_JAVA_OPTIONS}\"")) {
                modifyConfigState = FOUND;
                sb.append(line).append("\n");
                sb.append(OPENRASP_CONFIG);
                continue;
            }
            // 删除已经存在的配置项
            if (OPENRASP_REGEX.matcher(line).matches()) {
                continue;
            }
            sb.append(line).append("\n");
        }
        if (NOTFOUND == modifyConfigState) {
            throw new RaspError(E10001 + "set JAVA_OPTIONS=\"${SAVE_JAVA_OPTIONS}\"");
        }

        return sb.toString();
    }
}
