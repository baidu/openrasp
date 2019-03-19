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

package com.baidu.rasp.install;

import com.baidu.rasp.RaspError;
import org.apache.commons.io.IOUtils;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;

import static com.baidu.rasp.RaspError.E10002;
import static com.baidu.rasp.RaspError.E10004;

/**
 * Created by OpenRASP on 5/19/17.
 * All rights reserved
 */
public abstract class InstallerFactory {
    protected static final String TOMCAT = "Tomcat";
    protected static final String JBOSS = "JBoss 4-6";
    protected static final String RESIN = "Resin";
    protected static final String WEBLOGIC = "Weblogic";
    protected static final String JBOSSEAP = "JbossEAP";
    protected static final String WILDFLY = "Wildfly";

    protected abstract Installer getInstaller(String serverName, String serverRoot);

    public Installer getInstaller(File serverRoot) throws RaspError {
        if (!serverRoot.exists()) {
            throw new RaspError(E10002 + serverRoot.getPath());
        }

        String serverName = detectServerName(serverRoot.getAbsolutePath());
        if (serverName == null) {
            System.out.println("List of currently supported servers are:");
            System.out.println("- " + TOMCAT);
            System.out.println("- " + RESIN);
            System.out.println("-" + WEBLOGIC);
            System.out.println("- " + JBOSSEAP);
            System.out.println("-" + WILDFLY);
            System.out.println("- " + JBOSS + "\n");
            throw new RaspError(E10004 + serverRoot.getPath());
        }

        System.out.println("Detected application server type: " + serverName);
        return getInstaller(serverName, serverRoot.getAbsolutePath());
    }

    public static String detectServerName(String serverRoot) throws RaspError{
        if (new File(serverRoot, "bin/catalina.sh").exists()
                || new File(serverRoot, "bin/catalina.bat").exists()) {
            return TOMCAT;
        }
        if (new File(serverRoot, "bin/probe.sh").exists()
                || new File(serverRoot, "bin/probe.bat").exists()
                || new File(serverRoot, "bin/twiddle.sh").exists()
                || new File(serverRoot, "bin/twiddle.bat").exists()) {
            return JBOSS;
        }
        if (new File(serverRoot, "bin/httpd.sh").exists()
                || new File(serverRoot, "bin/resin.sh").exists()) {
            return RESIN;
        }
        if (new File(serverRoot, "bin/startWebLogic.sh").exists()
                || new File(serverRoot, "bin/startWebLogic.bat").exists()) {
            return WEBLOGIC;
        }
        if (new File(serverRoot, "bin/standalone.sh").exists()
                || new File(serverRoot, "bin/standalone.bat").exists()) {
            if (detectWildfly(serverRoot)) {
                return WILDFLY;
            } else {
                return JBOSSEAP;
            }
        }
        return null;
    }

    private static boolean detectWildfly(String severRoot) throws RaspError {
        String command = "./standalone.sh -v";
        File file = new File(severRoot + File.separator + "bin");
        if (file.exists() && file.isDirectory()) {
            try {
                Process p = Runtime.getRuntime().exec(command, null, file);
                String result = IOUtils.toString(p.getInputStream(), "UTF-8");
                if (result != null && result.toLowerCase().contains("wildfly")) {
                    return true;
                }
            } catch (IOException e) {
                throw new RaspError(E10004 + severRoot);
            }
        }
        return false;
    }
}
