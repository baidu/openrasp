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

import java.io.File;
import java.io.IOException;
import java.net.URISyntaxException;

/**
 * Created by OpenRASP on 5/11/17.
 */
public class App {

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

    private static void showBanner() {
        String banner = "OpenRASP Installer for Java app servers - Copyright 2017-2018 Baidu Inc.\n" +
                "For more details visit: https://rasp.baidu.com/doc/install/software.html\n";
        System.out.println(banner);
    }

    private static void showHelp() {
        String helpMsg = "Install:\n" +
                "java -jar RaspInstall.jar -install <path/to/server_home>\n" +
                "\n" +
                "Uninstall:\n" +
                "java -jar RaspInstall.jar -uninstall <path/to/server_home>\n";
        System.out.println(helpMsg);
    }

    private static void showArgs(String arg) {
        System.out.println("Bad argument " + arg + ", try again with -install or -uninstall");
    }

    public static void main(String[] args) throws IOException, URISyntaxException {
        showBanner();

        if (args.length < 2) {
            showHelp();
            return;
        }
        //参数0：rasp安装根目录
        //参数1：-install 安装，-uninstall 卸载
        try {
            if ("-install".equals(args[0])) {
                File serverRoot = new File(args[1]);
                InstallerFactory factory = newInstallerFactory();
                Installer installer = factory.getInstaller(serverRoot);
                installer.install();
            } else if ("-uninstall".equals(args[0])) {

                File serverRoot = new File(args[1]);
                UninstallerFactory factory = newUninstallerFactory();
                Uninstaller uninstaller = factory.getUninstaller(serverRoot);
                uninstaller.uninstall();

            } else {

                showArgs(args[0]);
            }
        } catch (Exception e) {
            System.out.println(e.getMessage() + "\n");
            // showHelp();
        }
    }
}
