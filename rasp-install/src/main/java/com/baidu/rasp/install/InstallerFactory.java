package com.baidu.rasp.install;

import com.baidu.rasp.RaspError;

import java.io.File;

import static com.baidu.rasp.RaspError.E10002;

/**
 * Created by OpenRASP on 5/19/17.
 * All rights reserved
 */
public abstract class InstallerFactory {
    protected static final String TOMCAT = "tomcat";
    protected static final String JBOSS = "jboss4-6";

    protected abstract Installer getInstaller(String serverName, String serverRoot);

    public Installer getInstaller(File serverRoot) throws RaspError {
        if (!serverRoot.exists()) {
            throw new RaspError(E10002 + serverRoot.getPath());
        }

        String serverName = detectServerName(serverRoot.getAbsolutePath());
        if (serverName == null) {
            System.out.println("Unable to determine app server type, list of currently supported servers are:");
            System.out.println("\t" + TOMCAT);
            System.out.println("\t" + JBOSS);
            return null;
        }
        
        System.out.println("Server type: " + serverName);
        return getInstaller(serverName, serverRoot.getAbsolutePath());
    }

    private static String detectServerName(String serverRoot) {
        if (new File(serverRoot, "bin/catalina.sh").exists()
                || new File(serverRoot, "bin/catalina.bat").exists()) {
            return TOMCAT;
        }
        if (new File(serverRoot, "bin/probe.sh").exists()
                || new File(serverRoot, "bin/probe.bat").exists()) {
            return JBOSS;
        }
        return null;
    }
}
