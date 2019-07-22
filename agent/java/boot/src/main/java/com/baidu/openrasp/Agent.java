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

package com.baidu.openrasp;

import org.apache.commons.cli.*;

import java.io.IOException;
import java.lang.instrument.Instrumentation;
import java.net.URL;
import java.util.jar.Attributes;
import java.util.jar.Manifest;

import static com.baidu.openrasp.Module.START_ACTION_INSTALL;
import static com.baidu.openrasp.Module.START_MODE_NORMAL;

/**
 * Created by tyy on 3/27/17.
 * 加载agent的入口类，先于主函数加载
 */
public class Agent {

    public static String projectVersion;
    public static String buildTime;
    public static String gitCommit;

    public static void main(String[] args) {
        try {
            Options options = new Options();
            options.addOption("h", "help", false, "print options information");
            options.addOption("v", "version", false, "print the version of rasp");
            HelpFormatter helpFormatter = new HelpFormatter();
            CommandLineParser parser = new DefaultParser();
            CommandLine cmd = parser.parse(options, args);
            if (cmd.hasOption("v")) {
                readVersion();
                System.out.println("Version:       " + projectVersion + "\n" +
                        "Build Time:    " + buildTime + "\n" +
                        "Git Commit ID: " + gitCommit);
            } else if (cmd.hasOption("h")) {
                helpFormatter.printHelp("java -jar rasp.jar", options, true);
            } else {
                helpFormatter.printHelp("java -jar rasp.jar", options, true);
            }
        } catch (Throwable e) {
            System.out.println("failed to parse options\n" + e.getMessage());
        }
    }

    /**
     * 启动时加载的agent入口方法
     *
     * @param agentArg 启动参数
     * @param inst     {@link Instrumentation}
     */
    public static void premain(String agentArg, Instrumentation inst) {
        init(START_MODE_NORMAL, START_ACTION_INSTALL, inst);
    }

    /**
     * attack 机制加载 agent
     *
     * @param agentArg 启动参数
     * @param inst     {@link Instrumentation}
     */
    public static void agentmain(String agentArg, Instrumentation inst) {
        init(Module.START_MODE_ATTACH, agentArg, inst);
    }

    /**
     * attack 机制加载 agent
     *
     * @param mode 启动模式
     * @param inst {@link Instrumentation}
     */
    public static synchronized void init(String mode, String action, Instrumentation inst) {
        try {
            JarFileHelper.addJarToBootstrap(inst);
            readVersion();
            ModuleLoader.load(mode, action, inst);
        } catch (Throwable e) {
            System.err.println("[OpenRASP] Failed to initialize, will continue without security protection.");
            e.printStackTrace();
        }
    }

    public static void readVersion() throws IOException {
        Class clazz = Agent.class;
        String className = clazz.getSimpleName() + ".class";
        String classPath = clazz.getResource(className).toString();
        String manifestPath = classPath.substring(0, classPath.lastIndexOf("!") + 1) + "/META-INF/MANIFEST.MF";
        Manifest manifest = new Manifest(new URL(manifestPath).openStream());
        Attributes attr = manifest.getMainAttributes();
        projectVersion = attr.getValue("Project-Version");
        buildTime = attr.getValue("Build-Time");
        gitCommit = attr.getValue("Git-Commit");

        projectVersion = (projectVersion == null ? "UNKNOWN" : projectVersion);
        buildTime = (buildTime == null ? "UNKNOWN" : buildTime);
        gitCommit = (gitCommit == null ? "UNKNOWN" : gitCommit);
    }

}