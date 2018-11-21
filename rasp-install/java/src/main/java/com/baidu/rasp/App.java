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

package com.baidu.rasp;

import com.baidu.rasp.install.Installer;
import com.baidu.rasp.install.InstallerFactory;
import com.baidu.rasp.install.linux.LinuxInstallerFactory;
import com.baidu.rasp.install.windows.WindowsInstallerFactory;
import com.baidu.rasp.uninstall.Uninstaller;
import com.baidu.rasp.uninstall.UninstallerFactory;
import com.baidu.rasp.uninstall.linux.LinuxUninstallerFactory;
import com.baidu.rasp.uninstall.windows.WindowsUninstallerFactory;
import org.apache.commons.cli.*;

import java.io.File;
import java.io.IOException;
import java.net.URISyntaxException;

import static com.baidu.rasp.RaspError.E10005;

/**
 * Created by OpenRASP on 5/11/17.
 */
public class App {
    public static String install;
    public static String appId;
    public static String appSecret;
    public static String baseDir;
    public static String url;

    private static InstallerFactory newInstallerFactory() {
        if (System.getProperty("os.name").startsWith("Windows")) {
            return new WindowsInstallerFactory();
        } else {
            return new LinuxInstallerFactory();
        }
    }

    private static UninstallerFactory newUninstallerFactory() {
        if (System.getProperty("os.name").startsWith("Windows")) {
            return new WindowsUninstallerFactory();
        } else {
            return new LinuxUninstallerFactory();
        }
    }

    private static void argsParser(String[] args) throws ParseException, RaspError {
        Options options = new Options();
        options.addOption("install", true, "install OpenRASP agent");
        options.addOption("uninstall", true, "uninstall OpenRASP agent");
        options.addOption("appid", true, "Value of cloud.appid");
        options.addOption("appsecret", true, "Value of cloud.appsecret");
        options.addOption("backendurl", true, "Value of cloud.backendurl");
        options.addOption("help", false, "helpMessage");
        CommandLineParser parser = new PosixParser();
        CommandLine cmd = parser.parse(options, args);
        if (cmd.hasOption("help")) {
            HelpFormatter hf = new HelpFormatter();
            hf.printHelp("Options", options);
        } else {
            if (cmd.hasOption("install") && cmd.hasOption("uninstall") || !cmd.hasOption("install") && !cmd.hasOption("uninstall")) {
                throw new RaspError(E10005 + "install and uninstall must only be selected one");
            } else if (cmd.hasOption("install")) {
                baseDir = cmd.getOptionValue("install");
                install = "install";
            } else {
                baseDir = cmd.getOptionValue("uninstall");
                install = "uninstall";
            }
            appId = cmd.getOptionValue("appid");
            appSecret = cmd.getOptionValue("appsecret");
            url = cmd.getOptionValue("backendurl");
            if (!(appId != null && appSecret != null && url != null || appId == null && appSecret == null && url == null)) {
                throw new RaspError(E10005 + "url and appId and appSecret must be set at the same time");
            }
        }
    }

    private static void showBanner() {
        String banner = "OpenRASP Installer for Java app servers - Copyright 2017-2018 Baidu Inc.\n" +
                "For more details visit: https://rasp.baidu.com/doc/install/software.html\n";
        System.out.println(banner);
    }

    private static void showHelp() {
        String helpMsg = "Usage: \n" +
                "  java -jar RaspInstall.jar -install /tomcat/\n" +
                "  java -jar RaspInstall.jar -uninstall /tomcat/\n" +
                "\n" +
                "Additional    command line arguments: \n" +
                "  -install      Specify application server path\n" +
                "  -uninstall    Specify application server path\n" +
                "  -appid        Value of cloud.appid\n" +
                "  -backendurl   Value of cloud.address\n" +
                "  -appsecret    Value of cloud.appsecret\n";
        System.out.println(helpMsg);
    }

    public static void main(String[] args) throws IOException, URISyntaxException {
        showBanner();
        try {
            argsParser(args);
            System.out.println(install+"===="+baseDir+"====="+appSecret+"====="+appId+"===="+url);
            if ("install".equals(install)) {
                File serverRoot = new File(baseDir);
                InstallerFactory factory = newInstallerFactory();
                Installer installer = factory.getInstaller(serverRoot);
                installer.install(url, appId, appSecret);
            } else if ("uninstall".equals(install)) {
                File serverRoot = new File(baseDir);
                UninstallerFactory factory = newUninstallerFactory();
                Uninstaller uninstaller = factory.getUninstaller(serverRoot);
                uninstaller.uninstall();
            } else {
                showHelp();
            }
        } catch (Exception e) {
            System.out.println("eeeeeeeeeeeeee");
            System.out.println(e.getMessage() + "\n");
            System.exit(1);
        }
    }
}
