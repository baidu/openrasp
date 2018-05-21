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

package com.baidu.rasp.uninstall.windows;

import com.baidu.rasp.uninstall.BaseStandardUninstaller;
import com.baidu.rasp.install.windows.ResinInstaller;

import java.io.File;
import java.util.Scanner;
import java.util.regex.Pattern;

/**
 * @author anyang
 * @Description:resin自动卸载
 * @date 2018/4/25 19:36
 *
 */
public class ResinUninstaller extends BaseStandardUninstaller {

    private static Pattern OPENRASP_REGEX_WINDOWS = Pattern.compile(".*(\\s*OPENRASP\\s*|\\s*<jvm-arg>.*\\\\rasp\\\\).*");
    private static Pattern OPENRASP_REGEX_LINUX = Pattern.compile(".*(\\s*OPENRASP\\s*|\\s*<jvm-arg>.*/rasp/).*");

    public ResinUninstaller(String serverName, String serverRoot) {
        super(serverName, serverRoot);
    }

    @Override
    protected String getInstallPath(String serverRoot) {
        return serverRoot + File.separator + "rasp";
    }

    @Override
    protected String getScript(String installPath) {

        if (ResinInstaller.getVersion(installPath) == 3) {

            return new File(installPath).getParent() + File.separator + "conf" + File.separator + "resin.conf";

        } else {

            return new File(installPath).getParent() + File.separator + "conf" + File.separator + "cluster-default.xml";
        }
    }

    @Override
    protected String recoverStartScript(String content) {

        StringBuilder sb = new StringBuilder();
        Scanner scanner = new Scanner(content);
        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            if (OPENRASP_REGEX_WINDOWS.matcher(line).matches() || OPENRASP_REGEX_LINUX.matcher(line).matches()) {
                continue;
            }
            sb.append(line).append(LINE_SEP);
        }

        return sb.toString();
    }
}
