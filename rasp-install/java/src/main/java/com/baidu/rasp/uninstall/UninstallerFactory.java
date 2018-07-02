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

package com.baidu.rasp.uninstall;


import com.baidu.rasp.RaspError;
import com.baidu.rasp.install.InstallerFactory;

import java.io.File;

import static com.baidu.rasp.RaspError.E10002;
import static com.baidu.rasp.RaspError.E10004;

/**
 * @Description:
 * @author anyang
 * @date 2018/4/25 19:38
 *
 */
public abstract class UninstallerFactory {

    protected static final String TOMCAT = "Tomcat";
    protected static final String JBOSS = "JBoss 4-6";
    protected static final String RESIN = "Resin";

    protected abstract Uninstaller getUninstaller(String serverName, String serverRoot);

    public Uninstaller getUninstaller(File serverRoot) throws RaspError {
        if (!serverRoot.exists()) {
            throw new RaspError(E10002 + serverRoot.getPath());
        }

        String serverName = InstallerFactory.detectServerName(serverRoot.getAbsolutePath());
        if (serverName == null) {
            System.out.println("List of currently supported servers are:");
            System.out.println("- " + TOMCAT);
            System.out.println("- " + RESIN);
            System.out.println("- " + JBOSS + "\n");
            throw new RaspError(E10004 + serverRoot.getPath());
        }

        System.out.println("Detected application server type: " + serverName);
        return getUninstaller(serverName, serverRoot.getAbsolutePath());
    }

}
