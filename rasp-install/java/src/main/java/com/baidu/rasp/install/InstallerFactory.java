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

import java.io.*;

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
            System.out.println("- " + WEBLOGIC);
            System.out.println("- " + JBOSSEAP);
            System.out.println("- " + WILDFLY);
            System.out.println("- " + JBOSS + "\n");
            throw new RaspError(E10004 + serverRoot.getPath());
        }

        System.out.println("Detected application server type: " + serverName);
        return getInstaller(serverName, serverRoot.getAbsolutePath());
    }

    public static String detectServerName(String serverRoot) {
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
            try {
                return isWildfly(serverRoot) ? WILDFLY : JBOSSEAP;
            } catch (Exception e) {
                return null;
            }
        }
        return null;
    }

    public static boolean isWildfly(String serverRoot) throws Exception {
        File dir = new File(serverRoot + File.separator + "bin" + File.separator + "init.d");
        if (dir.exists() && dir.isDirectory()) {
            File[] files = dir.listFiles(new FileFilter() {
                @Override
                public boolean accept(File file) {
                    return file.getName().contains("wildfly");
                }
            });
            return files != null && files.length > 0;
        } else {
            return detectWildfly(serverRoot);
        }
    }

    private static boolean detectWildfly(String severRoot) throws Exception {
        File baseDir = new File(severRoot);
        if (baseDir.exists() && baseDir.isDirectory()) {
            String path;
            try {
                path = baseDir.getCanonicalPath() + File.separator + "README.txt";
            } catch (IOException e) {
                path = baseDir.getAbsolutePath() + File.separator + "README.txt";
            }
            String content = IOUtils.toString(new FileReader(new File(path)));
            return content != null && content.toLowerCase().contains("wildfly");
        }
        return false;
    }
}
