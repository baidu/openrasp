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
import java.io.UnsupportedEncodingException;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLDecoder;
import java.util.regex.Pattern;

import static com.baidu.rasp.RaspError.E10005;
import static com.baidu.rasp.RaspError.E10007;

/**
 * Created by OpenRASP on 5/11/17.
 */
public class App {
    public static String install;
    public static String appId;
    public static String appSecret;
    public static String baseDir;
    public static String url;
    public static int pid;
    public static boolean isAttach = false;
    public static boolean keepConfig = false;

    public static final String REGEX_APPID = "^[a-z0-9]{40,40}$";
    public static final String REGEX_APPSECRET = "^[a-zA-Z0-9_-]{43,45}$";

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
        options.addOption("install", true, "specify application server path");
        options.addOption("uninstall", true, "specify application server path");
        options.addOption("appid", true, "Value of cloud.appid");
        options.addOption("appsecret", true, "Value of cloud.appsecret");
        options.addOption("backendurl", true, "Value of cloud.backendurl");
        options.addOption("keepconf", false, "If the parameter exists, reserved openrasp.yml");
        options.addOption("help", false, "print options information");
        options.addOption("h", false, "print options information");
        options.addOption("pid", true, "specify the pid that the rasp attach to");
        CommandLineParser parser = new PosixParser();
        CommandLine cmd = parser.parse(options, args);
        if (cmd.hasOption("help") || cmd.hasOption("h")) {
            showHelp();
        } else {
            if (cmd.hasOption("install") && cmd.hasOption("uninstall")) {
                throw new RaspError(E10005 + "Can't use -install and -uninstall simultaneously");
            } else {
                if (cmd.hasOption("install")) {
                    baseDir = cmd.getOptionValue("install");
                    install = "install";
                } else if (cmd.hasOption("uninstall")) {
                    baseDir = cmd.getOptionValue("uninstall");
                    install = "uninstall";
                } else {
                    throw new RaspError(E10005 + "One of -install and -uninstall must be specified");
                }

                if (cmd.hasOption("pid")) {
                    isAttach = true;
                    try {
                        pid = Integer.parseInt(cmd.getOptionValue("pid"));
                    } catch (NumberFormatException e) {
                        throw new RaspError(E10005 + "The -pid parameter must have a integer value");
                    }
                }
            }

            keepConfig = cmd.hasOption("keepconf");
            appId = cmd.getOptionValue("appid");
            appSecret = cmd.getOptionValue("appsecret");
            url = cmd.getOptionValue("backendurl");
            if (!(appId != null && appSecret != null && url != null || appId == null && appSecret == null && url == null)) {
                throw new RaspError(E10005 + "-backendurl, -appid and -appsecret must be set simultaneously");
            }
        }
    }

    private static void checkArgs() throws RaspError {
        if (appId != null) {
            Pattern pattern = Pattern.compile(REGEX_APPID);
            if (!pattern.matcher(appId).matches()) {
                throw new RaspError(E10005 + "appid must be exactly 40 characters in length");
            }
        }
        if (appSecret != null) {
            Pattern pattern = Pattern.compile(REGEX_APPSECRET);
            if (!pattern.matcher(appSecret).matches()) {
                throw new RaspError(E10005 + "appsecret must have 43~45 characters");
            }
        }
        if (url != null) {
            try {
                new URL(url);
            } catch (MalformedURLException e) {
                throw new RaspError(E10005 + "backendurl must be a valid URL, e.g http://192.168.1.1");
            }
        }
    }

    private static void showBanner() {
        String banner = "OpenRASP Installer for Java app servers - Copyright 2017-2019 Baidu Inc.\n" +
                "For more details visit: https://rasp.baidu.com/doc/install/software.html\n";
        System.out.println(banner);
    }

    private static void showNotice() {
        URL localUrl = App.class.getProtectionDomain().getCodeSource().getLocation();
        String notice;
        try {
            String path = URLDecoder.decode(localUrl.getFile().replace("+", "%2B"), "UTF-8");
            notice = "Try 'java -jar " + path + " -help' for more information.";
        } catch (UnsupportedEncodingException e) {
            notice = "Try 'java -jar <path/to/RaspInstall.jar> -help' for more information.";
        }
        System.out.println(notice);
    }

    private static void showHelp() {
        String helpMsg = "Usage: \n" +
                "  java -jar RaspInstall.jar -install /tomcat/\n" +
                "  java -jar RaspInstall.jar -uninstall /tomcat/\n" +
                "\n" +
                "Additional command line arguments: \n" +
                "  -install      Install OpenRASP\n" +
                "  -uninstall    Uninstall OpenRASP\n" +
                "  -appid        Value of cloud.appid\n" +
                "  -backendurl   Value of cloud.address\n" +
                "  -appsecret    Value of cloud.appsecret\n" +
                "  -keepconf     Do not overwrite openrasp.yml\n" +
                "  -help/-h      Show this dialog\n";
        System.out.println(helpMsg);
    }

    public static void operateServer(String[] args) throws RaspError, ParseException, IOException {
        showBanner();
        argsParser(args);
        checkArgs();
        if ("install".equals(install)) {
            File serverRoot = new File(baseDir);
            InstallerFactory factory = newInstallerFactory();
            Installer installer = factory.getInstaller(serverRoot);
            if (installer != null) {
                installer.install();
            } else {
                throw new RaspError(E10007);
            }

        } else if ("uninstall".equals(install)) {
            File serverRoot = new File(baseDir);
            UninstallerFactory factory = newUninstallerFactory();
            Uninstaller uninstaller = factory.getUninstaller(serverRoot);
            if (uninstaller != null) {
                uninstaller.uninstall();
            } else {
                throw new RaspError(E10007);
            }
        }
    }

    public static void main(String[] args) {
        try {
            operateServer(args);
        } catch (Exception e) {
            if (e instanceof RaspError) {
                System.out.println(e.getMessage());
            } else {
                e.printStackTrace();
            }
            showNotice();
            System.exit(1);
        }
    }
}
