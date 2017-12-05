package com.baidu.rasp.install;


import com.baidu.rasp.RaspError;

import java.io.IOException;

/**
 * Created by OpenRASP on 5/19/17.
 * All rights reserved
 */
public interface Installer {
    void install() throws RaspError, IOException;
}
