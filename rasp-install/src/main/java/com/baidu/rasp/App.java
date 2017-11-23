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

    private static void showHelp() {
        String helpMsg = 
            "OpenRASP Installer for Java app servers - Copyright ©2017 Baidu Inc.\n" + 
            "For more details visit: http://rasp.baidu.com/doc/install/software.html\n\n" +
            "Usage:\n" + 
            "java -jar RaspInstall.jar <path/to/server_home>";
        System.out.println(helpMsg);
    }

    public static void main(String[] args) throws IOException, URISyntaxException {
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
            System.out.println(e.getMessage());
            showHelp();
        }
    }
}
