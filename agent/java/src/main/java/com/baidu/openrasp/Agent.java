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

package com.baidu.openrasp;

import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.messaging.LogConfig;
import com.baidu.openrasp.plugin.checker.CheckerManager;
import com.baidu.openrasp.plugin.js.engine.JsPluginManager;
import com.baidu.openrasp.tool.JarFileHelper;
import com.baidu.openrasp.transformer.CustomClassTransformer;
import org.apache.log4j.Logger;

import java.io.IOException;
import java.lang.instrument.Instrumentation;
import java.lang.instrument.UnmodifiableClassException;
import java.net.URL;
import java.util.LinkedList;
import java.util.jar.Attributes;
import java.util.jar.Manifest;

/**
 * Created by tyy on 3/27/17.
 * 加载agent的入口类，先于主函数加载
 */
public class Agent {

    private static String projectVersion;
    private static String buildTime;
    private static String gitCommit;

    /**
     * 启动时加载的agent入口方法
     *
     * @param agentArg 启动参数
     * @param inst     {@link Instrumentation}
     */
    public static void premain(String agentArg, Instrumentation inst) {
        try {
            JarFileHelper.addJarToBootstrap(inst);
            if (!loadConfig(JarFileHelper.getLocalJarParentPath())) {
                return;
            }
            readVersion();
            // 初始化插件系统
            JsPluginManager.init();
            CheckerManager.init();
            initTransformer(inst);
            String message = "OpenRASP Initialized [" + projectVersion + " (build: GitCommit=" + gitCommit + " date="
                    + buildTime + ")]";
            System.out.println(message);
            Logger.getLogger(Agent.class.getName()).info(message);
            HookHandler.enableHook.set(true);
        } catch (Exception e) {
            System.out.println("[OpenRASP] Failed to initialize, will continue without security protection.");

            e.printStackTrace();
        }
    }

    /**
     * 初始化配置
     *
     * @return 配置是否成功
     */
    private static boolean loadConfig(String baseDir) throws IOException {
        LogConfig.completeLogConfig(baseDir);

        return true;
    }

    /**
     * 初始化类字节码的转换器
     *
     * @param inst 用于管理字节码转换器
     */
    private static void initTransformer(Instrumentation inst) throws UnmodifiableClassException {
        LinkedList<Class> retransformClasses = new LinkedList<Class>();
        CustomClassTransformer customClassTransformer = new CustomClassTransformer();
        inst.addTransformer(customClassTransformer, true);
        Class[] loadedClasses = inst.getAllLoadedClasses();
        for (Class clazz : loadedClasses) {
            for (final AbstractClassHook hook : customClassTransformer.getHooks()) {
                if (hook.isClassMatched(clazz.getName().replace(".", "/"))) {
                    if (inst.isModifiableClass(clazz) && !clazz.getName().startsWith("java.lang.invoke.LambdaForm")) {
                        retransformClasses.add(clazz);
                    }
                }
            }
        }
        // hook已经加载的类
        Class[] classes = new Class[retransformClasses.size()];
        retransformClasses.toArray(classes);
        if (classes.length > 0) {
            inst.retransformClasses(classes);
        }
    }

    private static void readVersion() throws IOException {
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
