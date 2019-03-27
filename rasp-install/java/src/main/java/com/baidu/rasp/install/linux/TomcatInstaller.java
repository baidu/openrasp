/*
 * Copyright 2017-2019 Baidu Inc.
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

package com.baidu.rasp.install.linux;

import com.baidu.rasp.RaspError;
import com.baidu.rasp.install.BaseStandardInstaller;

import java.util.Scanner;

import static com.baidu.rasp.RaspError.E10001;

/**
 * Created by OpenRASP on 5/11/17.
 */
public class TomcatInstaller extends BaseStandardInstaller {

    private static String OPENRASP_START_TAG = "### BEGIN OPENRASP - DO NOT MODIFY ###\n";
    private static String OPENRASP_END_TAG = "### END OPENRASP ###\n";
    private static String JAVA_AGENT_CONFIG = "\tJAVA_OPTS=\"-javaagent:${CATALINA_HOME}/rasp/rasp.jar ${JAVA_OPTS}\"\n";
    private static String JDK_JAVA_OPTIONS =
            "JDK_JAVA_OPTIONS=\"$JDK_JAVA_OPTIONS --add-opens=java.base/jdk.internal.loader=ALL-UNNAMED\"\n" +
                    "export JDK_JAVA_OPTIONS\n";

    public TomcatInstaller(String serverName, String serverRoot) {
        super(serverName, serverRoot);
    }

    @Override
    protected String getInstallPath(String serverRoot) {
        return serverRoot + "/rasp";

        // String catalineBase;
        // if (serverRoot == null) {
        //     String output = runCommand(new String[]{"bash", "-c", "ps -elf | grep catalina.base"});
        //     Pattern PATTERN = Pattern.compile("-Dcatalina\\.base=(\\S+)");
        //     Matcher m = PATTERN.matcher(output);
        //     catalineBase = (m.find() ? m.group(1) : null);
        // } else {
        //     catalineBase = serverRoot;
        // }
        // return catalineBase + "/rasp";
    }

    @Override
    protected String getScript(String installDir) {
        return installDir + "/../bin/catalina.sh";
    }

    @Override
    protected String modifyStartScript(String content) throws RaspError {
        boolean versionFlag = checkTomcatVersion();
        StringBuilder sb = new StringBuilder();
        Scanner scanner = new Scanner(content);
        int modifyConfigState = NOTFOUND;
        boolean isDelete = false;

        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();

            // 插入点: [ $1 = "start" ] 或者 [ $1 = "run" ]
            if (!line.startsWith("#") && (line.contains("\"$1\" = \"start\"") || line.contains("\"$1\" = \"run\""))) {
                modifyConfigState = FOUND;
                sb.append(line).append("\n");
                sb.append(OPENRASP_START_TAG);
                sb.append(JAVA_AGENT_CONFIG);
                //jdk版本8以上插入依赖包
                if (versionFlag) {
                    sb.append(JDK_JAVA_OPTIONS);
                }
                sb.append(OPENRASP_END_TAG);
                continue;
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
            throw new RaspError(E10001 + "[\"$1\" = \"start\"] or [\"$1\" = \"run\"]");
        }
        return sb.toString();
    }

}
