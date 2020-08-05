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

package com.baidu.rasp;

import com.sun.tools.attach.VirtualMachine;

import java.io.File;

/**
 * Created by tyy on 19-2-26.
 */
public class Attacher {

    public static final String MODE_INSTALL = "install";
    public static final String MODE_UNINSTALL = "uninstall";

    private String pid;

    private String baseDir;

    public Attacher(String pid, String baseDir) {
        this.pid = pid;
        this.baseDir = baseDir;
    }

    public void doAttach(String action) throws RaspError {
        try {
            VirtualMachine virtualMachine = VirtualMachine.attach(pid);
            virtualMachine.loadAgent(baseDir + File.separator + "rasp" + File.separator + "rasp.jar", action);
            virtualMachine.detach();
        } catch (Throwable t) {
            throw new RaspError(RaspError.E10006 + "Failed to attach Rasp.jar to server: " + t.getMessage());
        }

        if (MODE_UNINSTALL.equals(action)) {
            System.out.println("Success to uninstall the OpenRASP with Attach mode, the OpenRASP has no effect.");
        } else if (MODE_INSTALL.equals(action)) {
            System.out.println("Success to install the OpenRASP with Attach mode, the OpenRASP has taken effect");
        }

    }

}
