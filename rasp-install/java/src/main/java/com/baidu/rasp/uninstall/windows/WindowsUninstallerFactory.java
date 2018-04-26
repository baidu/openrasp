package com.baidu.rasp.uninstall.windows;

import com.baidu.rasp.uninstall.Uninstaller;
import com.baidu.rasp.uninstall.UninstallerFactory;

/**
　　* @Description:
　　* @author anyang
　　* @date 2018/4/25 19:36
　　*/
public class WindowsUninstallerFactory extends UninstallerFactory{

    @Override
    protected Uninstaller getUninstaller(String serverName, String serverRoot) {
        if (serverName.equals(TOMCAT)) {
            return new TomcatUnInstaller(serverName, serverRoot);
        }
        if (serverName.equals(JBOSS)) {
            return new Jboss4Uninstaller(serverName, serverRoot);
        }
        if (serverName.equals(RESIN)) {
            return new ResinUninstaller(serverName, serverRoot);
        }
        System.out.println("Invalid server name: " + serverName);
        return null;
    }
}


