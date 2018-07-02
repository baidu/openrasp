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

package com.baidu.rasp.uninstall.linux;

import com.baidu.rasp.uninstall.Uninstaller;
import com.baidu.rasp.uninstall.UninstallerFactory;
import com.baidu.rasp.uninstall.windows.ResinUninstaller;

/**
 * @author anyang
 * @Description:
 * @date 2018/4/25 19:34
 */
public class LinuxUninstallerFactory extends UninstallerFactory {

    @Override
    protected Uninstaller getUninstaller(String serverName, String serverRoot) {
        if (serverName.equals(TOMCAT)) {
            return new TomcatUninstaller(serverName, serverRoot);
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
