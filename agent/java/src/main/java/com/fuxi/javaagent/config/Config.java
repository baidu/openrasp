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

package com.fuxi.javaagent.config;

import org.apache.log4j.Logger;


import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.util.Arrays;
import java.util.Properties;


/**
 * Created by tyy on 3/27/17.
 * 项目配置类，通过解析conf/rasp.property文件来加载配置
 * 若没有找到配置文件使用默认值
 */
public class Config {

    public static final int REFLECTION_STACK_START_INDEX = 0;

    private static final String DEFAULT_PLUGIN_TIMEOUT = "100";
    private static final String DEFAULT_BODYSIZE = "4096";
    private static final String DEFAULT_REFLECTION_MAX_STACK = "100";
    private static final String DEFAULT_IGNORE = "";
    private static final String DEFAULT_ENFORCE_POLICY = "false";
    private static final String DEFAULT_BLOCK_URL = "https://rasp.baidu.com/blocked";
    private static final String DEFAULT_REFLECT_MONITOR_METHOD = "java.lang.Runtime.getRuntime,"
            + "java.lang.Runtime.exec,"
            + "java.lang.ProcessBuilder.start";
    private static final String DEFAULT_LOG_STACK_SIZE = "20";
    private static final String DEFAULT_READ_FILE_EXTENSION_REGEX = "^(gz|7z|xz|tar|rar|zip|sql|db)$";

    private static final Logger LOGGER = Logger.getLogger(Config.class.getName());
    private static String baseDirectory;


    private int reflectionMaxStack;
    private long pluginTimeout;
    private long bodyMaxBytes;
    private String[] ignoreHooks;
    private boolean enforcePolicy;
    private String[] reflectionMonitorMethod;
    private int logMaxStackSize;
    private String readFileExtensionRegex;

    private String blockUrl;

    private enum KeyName {
        plugintimeoutmillis,
        bodymaxbytes,
        hooksignore,
        reflectionmaxstack,
        reflectionmonitor,
        blockurl,
        logmaxstack,
        securityenforce_policy,
        readfileextensionregex
    }

    // Config是由bootstrap classloader加载的，不能通过getProtectionDomain()的方法获得JAR路径
    static {
        Class clazz = Config.class;
        // path值示例：　file:/opt/apache-tomcat-xxx/rasp/rasp.jar!/com/fuxi/javaagent/Agent.class
        String path = clazz.getResource("/" + clazz.getName().replace(".", "/") + ".class").getPath();
        if (path.startsWith("file:")) {
            path = path.substring(5);
        }
        if (path.contains("!")) {
            path = path.substring(0, path.indexOf("!"));
        }
        try {
            baseDirectory = URLDecoder.decode(new File(path).getParent(), "UTF-8");
        } catch (UnsupportedEncodingException e) {
            LOGGER.warn(e.getMessage());
            baseDirectory = new File(path).getParent();
        }
    }

    /**
     * 构造函数，初始化全局配置
     */
    private Config() {
        FileInputStream input = null;

        this.pluginTimeout = Long.parseLong(DEFAULT_PLUGIN_TIMEOUT);
        this.bodyMaxBytes = Long.parseLong(DEFAULT_BODYSIZE);
        this.ignoreHooks = new String[]{};
        this.enforcePolicy = Boolean.parseBoolean(DEFAULT_ENFORCE_POLICY);
        this.reflectionMonitorMethod = DEFAULT_REFLECT_MONITOR_METHOD.replace(" ", "").split(",");
        this.reflectionMaxStack = Integer.parseInt(DEFAULT_REFLECTION_MAX_STACK);
        this.logMaxStackSize = Integer.parseInt(DEFAULT_LOG_STACK_SIZE);
        this.blockUrl = DEFAULT_BLOCK_URL;
        this.readFileExtensionRegex = DEFAULT_READ_FILE_EXTENSION_REGEX;

        try {
            input = new FileInputStream(new File(baseDirectory, "conf" + File.separator + "rasp.properties"));
            Properties properties = new Properties();
            properties.load(input);

            this.pluginTimeout = Long.parseLong(properties.getProperty("plugin.timeout.millis", DEFAULT_PLUGIN_TIMEOUT));
            this.bodyMaxBytes = Long.parseLong(properties.getProperty("body.maxbytes", DEFAULT_BODYSIZE));
            this.enforcePolicy = Boolean.parseBoolean(properties.getProperty("security.enforce_policy", DEFAULT_ENFORCE_POLICY));
            this.ignoreHooks = properties.getProperty("hooks.ignore", DEFAULT_IGNORE).replace(" ", "").split(",");
            this.reflectionMonitorMethod = properties.getProperty("reflection.monitor", DEFAULT_REFLECT_MONITOR_METHOD)
                    .replace(" ", "").split(",");
            this.reflectionMaxStack = Integer.parseInt(properties.getProperty("reflection.maxstack", DEFAULT_REFLECTION_MAX_STACK));
            this.logMaxStackSize = Integer.parseInt(properties.getProperty("log.maxstack", DEFAULT_LOG_STACK_SIZE));
            this.blockUrl = properties.getProperty("block.url", DEFAULT_BLOCK_URL);
            this.readFileExtensionRegex = properties.getProperty("readfile.extension.regex", DEFAULT_READ_FILE_EXTENSION_REGEX);
            if (this.blockUrl == null || blockUrl.equals("")) {
                this.blockUrl = DEFAULT_BLOCK_URL;
            }
        } catch (FileNotFoundException e) {
            LOGGER.warn("Could not find rasp.properties, using default settings: " + e.getMessage());
        } catch (IOException e) {
            LOGGER.warn("cannot load properties file: " + e.getMessage());
        } finally {
            try {
                input.close();
            } catch (Exception ignored) {
                // ignore
            }
        }

        for (int i = 0; i < this.ignoreHooks.length; i++) {
            this.ignoreHooks[i] = this.ignoreHooks[i].trim();
        }

        LOGGER.info("baseDirectory: " + baseDirectory);
        LOGGER.info("plugin.timeout.millis: " + pluginTimeout);
        LOGGER.info("hooks.ignore: " + Arrays.toString(this.ignoreHooks));
        LOGGER.info("reflection.monitor: " + Arrays.toString(this.reflectionMonitorMethod));
        LOGGER.info("reflection.maxstack: " + reflectionMaxStack);
        LOGGER.info("block.url: " + blockUrl);
        LOGGER.info("readfile.extension.regex: " + readFileExtensionRegex);
    }

    private static class ConfigHolder {
        static Config instance = new Config();
    }

    /**
     * 获取配置单例
     *
     * @return Config单例对象
     */
    public static Config getConfig() {
        return ConfigHolder.instance;
    }

    /**
     * 获取当前jar包所在目录
     *
     * @return 当前jar包目录
     */
    public String getBaseDirectory() {
        return baseDirectory;
    }

    /**
     * 获取js脚本所在目录
     *
     * @return js脚本目录
     */
    public String getScriptDirectory() {
        return baseDirectory + "/plugins";
    }


    //--------------------可以通过插件修改的配置项-----------------------------------

    /**
     * 获取Js引擎执行超时时间
     *
     * @return 超时时间
     */
    public synchronized long getPluginTimeout() {
        return pluginTimeout;
    }

    /**
     * 配置Js引擎执行超时时间
     *
     * @param pluginTimeout
     */
    public synchronized void setPluginTimeout(String pluginTimeout) {
        this.pluginTimeout = Long.parseLong(pluginTimeout);
    }

    /**
     * 保存HTTP请求体时最大保存长度
     *
     * @return 最大长度
     */
    public synchronized long getBodyMaxBytes() {
        return bodyMaxBytes;
    }

    /**
     * 配置保存HTTP请求体时最大保存长度
     *
     * @param bodyMaxBytes
     */
    public synchronized void setBodyMaxBytes(String bodyMaxBytes) {
        this.bodyMaxBytes = Long.parseLong(bodyMaxBytes);
    }

    /**
     * 需要忽略的挂钩点
     *
     * @return 需要忽略的挂钩点列表
     */
    public synchronized String[] getIgnoreHooks() {
        return this.ignoreHooks;
    }

    /**
     * 配置需要忽略的挂钩点
     *
     * @param ignoreHooks
     */
    public synchronized void setIgnoreHooks(String ignoreHooks) {
        this.ignoreHooks = ignoreHooks.replace(" ", "").split(",");
    }

    /**
     * 反射hook点传递给插件栈信息的最大深度
     *
     * @return 栈信息最大深度
     */
    public synchronized int getReflectionMaxStack() {
        return reflectionMaxStack;
    }

    /**
     * 设置反射hook点传递给插件栈信息的最大深度
     *
     * @param reflectionMaxStack 栈信息最大深度
     */
    public synchronized void setReflectionMaxStack(String reflectionMaxStack) {
        this.reflectionMaxStack = Integer.parseInt(reflectionMaxStack);
    }

    /**
     * 获取反射监控的方法
     *
     * @return 需要监控的反射方法
     */
    public synchronized String[] getReflectionMonitorMethod() {
        return reflectionMonitorMethod;
    }

    /**
     * 设置反射监控的方法
     *
     * @param reflectionMonitorMethod 监控的方法
     */
    public synchronized void setReflectionMonitorMethod(String reflectionMonitorMethod) {
        this.reflectionMonitorMethod = reflectionMonitorMethod.replace(" ", "").split(",");
    }

    /**
     * 获取拦截自定义页面的url
     *
     * @return 拦截页面url
     */
    public synchronized String getBlockUrl() {
        return blockUrl;
    }

    /**
     * 设置拦截页面url
     *
     * @param blockUrl 拦截页面url
     */
    public synchronized void setBlockUrl(String blockUrl) {
        this.blockUrl = blockUrl;
    }

    /**
     * 获取报警日志最大输出栈深度
     *
     * @return
     */
    public synchronized int getLogMaxStackSize() {
        return logMaxStackSize;
    }

    /**
     * 配置报警日志最大输出栈深度
     *
     * @param logMaxStackSize
     */
    public synchronized void setLogMaxStackSize(String logMaxStackSize) {
        this.logMaxStackSize = Integer.parseInt(logMaxStackSize);
    }

    /**
     * 是否开启强制安全规范
     * 如果开启检测有安全风险的情况下将会禁止服务器启动
     * 如果关闭当有安全风险的情况下通过日志警告
     *
     * @return true开启，false关闭
     */
    public synchronized boolean getEnforcePolicy() {
        return enforcePolicy;
    }

    /**
     * 配置是否开启强制安全规范
     *
     * @return true开启，false关闭
     */
    public synchronized void setEnforcePolicy(String enforcePolicy) {
        this.enforcePolicy = Boolean.parseBoolean(enforcePolicy);
    }

    /**
     * 获取读文件需要检测的扩展名正则表达式
     *
     * @return
     */
    public synchronized String getReadFileExtensionRegex() {
        return readFileExtensionRegex;
    }

    /**
     * 设置读文件需要检测的扩展名正则表达式
     *
     * @param readFileExtensionRegex
     */
    public synchronized void setReadFileExtensionRegex(String readFileExtensionRegex) {
        this.readFileExtensionRegex = readFileExtensionRegex;
    }

    //--------------------------统一的配置处理------------------------------------

    public boolean setConfig(String name, String value) {
        try {
            KeyName keyName = KeyName.valueOf(name.replace(".", ""));
            switch (keyName) {
                case plugintimeoutmillis:
                    setPluginTimeout(value);
                    break;
                case bodymaxbytes:
                    setBodyMaxBytes(value);
                    break;
                case hooksignore:
                    setIgnoreHooks(value);
                    break;
                case reflectionmaxstack:
                    setReflectionMaxStack(value);
                    break;
                case reflectionmonitor:
                    setReflectionMonitorMethod(value);
                    break;
                case blockurl:
                    setBlockUrl(value);
                    break;
                case logmaxstack:
                    setLogMaxStackSize(value);
                    break;
                case securityenforce_policy:
                    setEnforcePolicy(value);
                    break;
                case readfileextensionregex:
                    setReadFileExtensionRegex(value);
                    break;
                default:
                    // do nothing
            }
        } catch (Exception e) {
            LOGGER.info(e.getMessage());
            return false;
        }
        return true;
    }
}
