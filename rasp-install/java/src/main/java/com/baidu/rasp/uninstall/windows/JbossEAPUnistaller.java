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

package com.baidu.rasp.uninstall.windows;

import com.baidu.rasp.RaspError;
import com.baidu.rasp.uninstall.BaseStandardUninstaller;

/**
 * @description: jbossEAP自动化卸载
 * @author: anyang
 * @create: 2019/04/25 19:39
 */
public class JbossEAPUnistaller extends BaseStandardUninstaller {
    public JbossEAPUnistaller(String serverName, String serverRoot) {
        super(serverName, serverRoot);
    }

    @Override
    protected String getInstallPath(String serverRoot) {
        return serverRoot + "\\rasp";
    }

    @Override
    protected String getScript(String installPath) {
        return installPath + "\\..\\bin\\standalone.bat";
    }

    @Override
    protected String recoverStartScript(String content) throws RaspError {
        return getUninstallContent(content);
    }
}
