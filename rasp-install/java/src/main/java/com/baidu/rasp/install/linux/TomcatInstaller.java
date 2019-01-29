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
import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;

import java.io.*;
import java.util.Scanner;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import static com.baidu.rasp.RaspError.E10001;

/**
 * Created by OpenRASP on 5/11/17.
 */
public class TomcatInstaller extends BaseStandardInstaller {

    private static String OPENRASP_CONFIG =
        "### BEGIN OPENRASP - DO NOT MODIFY ###\n" +
        "\tJAVA_OPTS=\"-javaagent:${CATALINA_HOME}/rasp/rasp.jar ${JAVA_OPTS}\"\n" +
        "### END OPENRASP ###\n";
    private static String JDK_JAVA_OPTIONS = "JDK_JAVA_OPTIONS=\"$JDK_JAVA_OPTIONS --add-opens=java.base/jdk.internal.loader=ALL-UNNAMED\"\n";
    private static Pattern OPENRASP_REGEX = Pattern.compile(".*(\\s*OPENRASP\\s*|JAVA_OPTS.*/rasp/).*");
    private static Pattern JDK_JAVA_OPTIONS_REGEX = Pattern.compile("^JDK_JAVA_OPTIONS.*jdk\\.internal\\.loader.*");

    TomcatInstaller(String serverName, String serverRoot) {
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
        boolean jdk_java_options = false;
        StringBuilder sb = new StringBuilder();
        Scanner scanner = new Scanner(content);
        int modifyConfigState = NOTFOUND;

        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();

            // 插入点: [ $1 = "start" ] 或者 [ $1 = "run" ]
            if (!line.startsWith("#") && (line.contains("\"$1\" = \"start\"") || line.contains("\"$1\" = \"run\""))) {
                modifyConfigState = FOUND;
                sb.append(line).append("\n");
                sb.append(OPENRASP_CONFIG);
                continue;
            }

            //添加jdk9以上的版本的依赖
            if (line.startsWith("JDK_JAVA_OPTIONS=") && !line.contains("export") && versionFlag && !jdk_java_options) {
                jdk_java_options = true;
                sb.append(line).append("\n");
                sb.append(JDK_JAVA_OPTIONS);
                continue;
            }

            // 删除已经存在的配置项
            if (OPENRASP_REGEX.matcher(line).matches() || JDK_JAVA_OPTIONS_REGEX.matcher(line).matches()) {
                continue;
            }

            sb.append(line).append("\n");
        }

        if (NOTFOUND == modifyConfigState) {
            throw new RaspError(E10001 + "[\"$1\" = \"start\"] or [\"$1\" = \"run\"]");
        }

        if (versionFlag && !jdk_java_options) {
            throw new RaspError(E10001 + "JDK_JAVA_OPTIONS=");
        }
        return sb.toString();
    }

}
