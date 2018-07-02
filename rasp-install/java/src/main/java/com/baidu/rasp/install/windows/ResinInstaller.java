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

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;
import java.util.Scanner;
import java.util.regex.Pattern;

import static com.baidu.rasp.RaspError.E10001;

/**
　　* @Description: resin自动安装
　　* @author anyang
　　* @date 2018/4/25 19:31
　　*/
public class ResinInstaller extends BaseStandardInstaller {

    private static Pattern OPENRASP_REGEX_WINDOWS = Pattern.compile(".*(\\s*OPENRASP\\s*|\\s*<jvm-arg>.*\\\\rasp\\\\).*");
    private static Pattern OPENRASP_REGEX_LINUX = Pattern.compile(".*(\\s*OPENRASP\\s*|\\s*<jvm-arg>.*/rasp/).*");
    public ResinInstaller(String serverName, String serverRoot) {
        super(serverName, serverRoot);
    }

    public String getOpenRASPConfig() {
        String configStart ="\t<!-- BEGIN OPENRASP - DO NOT MODIFY -->";
        String configEnd = "\t<!-- END OPENRASP -->";
        String path1 = "\t<jvm-arg>-javaagent:" +BaseStandardInstaller.resinPath + File.separator + "rasp" + File.separator + "rasp.jar</jvm-arg>";
        String path2 = "\t<jvm-arg>-Dlog4j.rasp.configuration=file:" + BaseStandardInstaller.resinPath + File.separator + "rasp" + File.separator + "conf" + File.separator + "rasp-log4j.xml</jvm-arg>";

        return configStart + LINE_SEP + path1 + LINE_SEP + path2 + LINE_SEP + configEnd + LINE_SEP;
    }

    public static int getVersion(String installPath) {

        String command = "java -classpath ./resin.jar com.caucho.Version";
        int version = -1;
        try {
            File resinFile = new File(new File(installPath).getParent() + File.separator + "lib");
            Process p = null;
            p = Runtime.getRuntime().exec(command, null, resinFile);
            InputStreamReader reader = new InputStreamReader(p.getInputStream(), "GBK");
            BufferedReader bufferedReader = new BufferedReader(reader);
            StringBuilder sb = new StringBuilder();
            String temp = "";
            while ((temp = bufferedReader.readLine()) != null) {
                sb.append(temp);
            }
            p.destroy();
            bufferedReader.close();
            reader.close();
            String res = new String(sb).split("\\.")[0];
            sb.delete(0, sb.length());
            for (int i = res.length() - 1; i >= 0; i--) {
                if (res.charAt(i) >= '0' && res.charAt(i) <= '9') {
                    sb.append(res.charAt(i));
                } else {

                    break;
                }
            }
            version = Integer.valueOf(sb.reverse().toString());

        } catch (Exception e) {

            e.printStackTrace();
        }
        return version;

    }

    @Override
    protected String getInstallPath(String serverRoot) {
        return serverRoot + File.separator + "rasp";
    }

    @Override
    protected String modifyStartScript(String content) throws RaspError {
        int modifyConfigState = NOTFOUND;
        StringBuilder sb = new StringBuilder();
        Scanner scanner = new Scanner(content);
        String OPENRASP_CONFIG = getOpenRASPConfig();
        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            if (FOUND == modifyConfigState) {
                sb.append(OPENRASP_CONFIG);
                modifyConfigState = DONE;
            }
            if (DONE == modifyConfigState) {
                if (OPENRASP_REGEX_WINDOWS.matcher(line).matches() || OPENRASP_REGEX_LINUX.matcher(line).matches()) {
                    continue;
                }
            }
            if (line.endsWith("<server-default>") && NOTFOUND == modifyConfigState) {
                modifyConfigState = FOUND;
            }
            sb.append(line).append(LINE_SEP);
        }
        if (NOTFOUND == modifyConfigState) {
            throw new RaspError(E10001 + "<server-default>");
        }
        return sb.toString();
    }

    @Override
    protected String getScript(String installPath) {

        if (getVersion(installPath) == 3) {

            return new File(installPath).getParent() + File.separator + "conf" + File.separator + "resin.conf";

        } else {

            return new File(installPath).getParent() + File.separator + "conf" + File.separator + "cluster-default.xml";
        }
    }
}
