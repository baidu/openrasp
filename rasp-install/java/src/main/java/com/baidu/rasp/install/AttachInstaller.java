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
import com.sun.tools.attach.VirtualMachine;

import java.io.File;

/**
 * Created by tyy on 19-1-22.
 */
public class AttachInstaller implements Installer {

    public static final String MODE_INSTALL = "install";

    private String pid;

    private String baseDir;

    public AttachInstaller(String pid, String baseDir) {
        this.pid = pid;
        this.baseDir = baseDir;
    }

    @Override
    public void install() throws RaspError {
        try {
            VirtualMachine virtualMachine = VirtualMachine.attach(pid);
            virtualMachine.loadAgent(baseDir + File.separator + "rasp" + File.separator + "rasp.jar", MODE_INSTALL);
            virtualMachine.detach();
        } catch (Throwable t) {
            t.printStackTrace();
            throw new RaspError(RaspError.E10006 + "Failed to attach Rasp.jar to server: " + t.getMessage());
        }
    }

    String getPid() {
        return pid;
    }

}
