package com.baidu.rasp.uninstall.linux;

import com.baidu.rasp.uninstall.Uninstaller;
import com.baidu.rasp.uninstall.UninstallerFactory;
import com.baidu.rasp.uninstall.windows.ResinUninstaller;

/**
　　* @Description:
　　* @author anyang
　　* @date 2018/4/25 19:34
　　*/
public class LinuxUninstallerFactory extends UninstallerFactory{

    @Override
    protected Uninstaller getUninstaller(String serverName, String serverRoot) {
        if (serverName.equals(TOMCAT)){
            return new TomcatUninstaller(serverName,serverRoot);
        }
        if (serverName.equals(JBOSS)){
            return new Jboss4Uninstaller(serverName,serverRoot);
        }
        if (serverName.equals(RESIN)) {
            return new ResinUninstaller(serverName, serverRoot);
        }
        System.out.println("Invalid server name: " + serverName);
        return null;
    }
}
