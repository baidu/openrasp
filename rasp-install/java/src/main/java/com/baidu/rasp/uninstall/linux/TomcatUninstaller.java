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

package com.baidu.rasp.uninstall.linux;

import com.baidu.rasp.uninstall.BaseStandardUninstaller;

import java.io.File;
import java.util.Scanner;
import java.util.regex.Pattern;

/**
 * @author anyang
 * @Description: tomcat自动卸载
 * @date 2018/4/25 19:34
 */
public class TomcatUninstaller extends BaseStandardUninstaller {

    private static Pattern OPENRASP_REGEX = Pattern.compile(".*(\\s*OPENRASP\\s*|JAVA_OPTS.*/rasp/).*");

    public TomcatUninstaller(String serverName, String serverRoot) {
        super(serverName, serverRoot);
    }

    @Override
    protected String getInstallPath(String serverRoot) {
        return serverRoot + "/rasp/";
    }

    @Override
    protected String getScript(String installPath) {

        return new File(installPath).getParent() + File.separator + "bin" + File.separator + "catalina.sh";
    }

    @Override
    protected String recoverStartScript(String content) {
        StringBuilder sb = new StringBuilder();
        Scanner scanner = new Scanner(content);
        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            if (OPENRASP_REGEX.matcher(line).matches()) {
                continue;
            }
            sb.append(line).append(LINE_SEP);
        }

        return sb.toString();
    }
}
