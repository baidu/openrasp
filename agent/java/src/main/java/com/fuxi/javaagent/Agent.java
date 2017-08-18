/**
 * Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

package com.fuxi.javaagent;

import com.fuxi.javaagent.messaging.LogConfig;
import com.fuxi.javaagent.plugin.PluginManager;
import com.fuxi.javaagent.tool.JarFileHelper;
import com.fuxi.javaagent.transformer.CustomClassTransformer;
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
            if (!LogConfig.completeLogConfig(JarFileHelper.getLocalJarParentPath())) {
                return;
            }
            readVersion();
            initTransformer(inst);
            // 初始化插件系统
            PluginManager.init();
            String message = "OpenRasp Initialized [" + projectVersion + " (build: GitCommit=" + gitCommit + " date="
                    + buildTime + ")]";
            System.out.println(message);
            Logger.getLogger(Agent.class.getName()).info(message);
            HookHandler.enableHook.set(true);
        } catch (Exception e) {
            System.out.println("init agent fail:" + e.getMessage() + "\n"
                    + "The program continues to run.");
        }
    }

    /**
     * 初始化类字节码的转换器
     *
     * @param inst 用于管理字节码转换器
     */
    private static void initTransformer(Instrumentation inst) throws UnmodifiableClassException {
        LinkedList<Class> retransformClasses = new LinkedList<Class>();
        inst.addTransformer(new CustomClassTransformer(), true);
        Class[] loadedClasses = inst.getAllLoadedClasses();
        for (Class clazz : loadedClasses) {
            if (inst.isModifiableClass(clazz) && !clazz.getName().startsWith("java.lang.invoke.LambdaForm")) {
                retransformClasses.add(clazz);
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
