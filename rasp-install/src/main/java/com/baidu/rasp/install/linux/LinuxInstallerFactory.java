package com.baidu.rasp.install.linux;

import com.baidu.rasp.install.Installer;
import com.baidu.rasp.install.InstallerFactory;

/**
 * Created by OpenRASP on 5/19/17.
 * All rights reserved
 */
public class LinuxInstallerFactory extends InstallerFactory {
    @Override
    protected Installer getInstaller(String serverName, String serverRoot) {
        if (serverName.equals(TOMCAT)) {
            return new TomcatInstaller(serverName, serverRoot);
        }
        if (serverName.equals(JBOSS)) {
            return new Jboss4Installer(serverName, serverRoot);
        }
        System.out.println("Invalid server name: " + serverName);
        return null;
    }
}
