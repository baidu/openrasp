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

package com.baidu.rasp.install.windows;

import com.baidu.rasp.RaspError;
import com.baidu.rasp.install.BaseStandardInstaller;

import java.util.Scanner;
import java.util.regex.Pattern;

import static com.baidu.rasp.RaspError.E10001;

/**
 * Created by OpenRASP on 5/15/17.
 */
public class TomcatInstaller extends BaseStandardInstaller {

    private static String OPENRASP_START_TAG = "rem BEGIN OPENRASP - DO NOT MODIFY" + LINE_SEP;
    private static String OPENRASP_END_TAG = "rem END OPENRASP" + LINE_SEP;
    private static String OPENRASP_CONFIG = "if \"%ACTION%\" == \"start\" set JAVA_OPTS=\"-javaagent:%CATALINA_HOME%\\rasp\\rasp.jar\" %JAVA_OPTS%" + LINE_SEP;
    private static String JDK_JAVA_OPTIONS = "set \"JDK_JAVA_OPTIONS=%JDK_JAVA_OPTIONS% --add-opens=java.base/jdk.internal.loader=ALL-UNNAMED\"" + LINE_SEP;

    private static Pattern OPENRASP_REGEX = Pattern.compile(".*(\\s*OPENRASP\\s*|JAVA_OPTS.*\\\\rasp\\\\).*");
    private static Pattern JDK_JAVA_OPTIONS_REGEX = Pattern.compile(".*JDK_JAVA_OPTIONS.*jdk\\.internal\\.loader.*");


    public TomcatInstaller(String serverName, String serverRoot) {
        super(serverName, serverRoot);
    }

    @Override
    protected String getInstallPath(String serverRoot) {
        return serverRoot + "\\rasp";
    }

    @Override
    protected String getScript(String installPath) {
        return installPath + "\\..\\bin\\catalina.bat";
    }

    @Override
    protected String modifyStartScript(String content) throws RaspError {
        boolean versionFlag = checkTomcatVersion();
        int modifyConfigState = NOTFOUND;
        StringBuilder sb = new StringBuilder();
        Scanner scanner = new Scanner(content);
        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            if (FOUND == modifyConfigState) {
                sb.append(OPENRASP_START_TAG);
                sb.append(OPENRASP_CONFIG);
                //jdk版本大于8加入依赖包
                if (versionFlag) {
                    sb.append(JDK_JAVA_OPTIONS);
                }
                sb.append(OPENRASP_END_TAG);
                modifyConfigState = DONE;
            }
            if (DONE == modifyConfigState) {
                if (OPENRASP_REGEX.matcher(line).matches() || JDK_JAVA_OPTIONS_REGEX.matcher(line).matches()) {
                    continue;
                }
            }
            if (line.startsWith(":setArgs") && NOTFOUND == modifyConfigState) {
                modifyConfigState = FOUND;
            }
            sb.append(line).append(LINE_SEP);
        }
        if (NOTFOUND == modifyConfigState) {
            throw new RaspError(E10001 + "\":setArgs\"");
        }
        return sb.toString();
    }

}
