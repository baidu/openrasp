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

    private static void showBanner() {
        String banner = "OpenRASP Installer for Java app servers - Copyright ©2017 Baidu Inc.\n" + 
            "For more details visit: https://rasp.baidu.com/doc/install/software.html\n";
        System.out.println(banner);
    }

    private static void showHelp() {
        String helpMsg = 
            "Usage:\n" + 
            "java -jar RaspInstall.jar <path/to/server_home>";
        System.out.println(helpMsg);
    }

    public static void main(String[] args) throws IOException, URISyntaxException {
        showBanner();

        if (args.length < 1) {
            showHelp();
            return;
        }

        try {
            File serverRoot = new File(args[0]);
            InstallerFactory factory = newInstallerFactory();
            Installer installer = factory.getInstaller(serverRoot);
            installer.install();
        } catch (Exception e) {
            System.out.println(e.getMessage() + "\n");
            // showHelp();
        }
    }
}
