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

package com.baidu.openrasp.detector;

import com.baidu.openrasp.HookHandler;
import org.apache.log4j.Logger;

import java.security.ProtectionDomain;
import java.util.HashSet;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Created by tyy on 19-2-12.
 */
public class ServerDetectorManager {

    private static final Logger LOGGER = Logger.getLogger(ServerDetectorManager.class.getName());

    private static final ServerDetectorManager instance = new ServerDetectorManager();
    private HashSet<ServerDetector> detectors = new HashSet<ServerDetector>();

    private ServerDetectorManager() {
        detectors.add(new TomcatDetector());
        detectors.add(new JBossDetector());
        detectors.add(new JBossEAPDetector());
        detectors.add(new JettyDetector());
        detectors.add(new WeblogicDetector());
        detectors.add(new ResinDetector());
        detectors.add(new WebsphereDetector());
        detectors.add(new UndertowDetector());
        detectors.add(new DubboDetector());
    }

    public static ServerDetectorManager getInstance() {
        return instance;
    }

    /**
     * 探测类是否为服务器的标志类
     *
     * @param className   类名
     * @param classLoader 该类的加载器
     */
    public synchronized void detectServer(String className, ClassLoader classLoader, ProtectionDomain domain) {
        try {
            for (ServerDetector detector : detectors) {
                if (detector.isClassMatched(className) && detector.handleServer(className, classLoader, domain)) {
                    HookHandler.LOGGER.info("detect server class: " + className);
                }
            }
        } catch (Throwable e) {
            e.printStackTrace();
            LOGGER.warn("detect class " + className + " failed", e);
        }
    }

    public boolean isClassMatched(String className) {
        for (ServerDetector detector : detectors) {
            if (detector.isClassMatched(className)) {
                return true;
            }
        }
        return false;
    }

}
