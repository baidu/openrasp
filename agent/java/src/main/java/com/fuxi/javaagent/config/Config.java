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

import com.fuxi.javaagent.contentobjects.jnotify.JNotifyException;
import com.fuxi.javaagent.exception.ConfigLoadException;
import com.fuxi.javaagent.tool.filemonitor.FileScanListener;
import com.fuxi.javaagent.tool.filemonitor.FileScanMonitor;
import org.apache.commons.lang3.StringUtils;
import org.apache.log4j.Logger;

import java.io.*;
import java.net.URLDecoder;
import java.util.Properties;


/**
 * Created by tyy on 3/27/17.
 * 项目配置类，通过解析conf/rasp.property文件来加载配置
 * 若没有找到配置文件使用默认值
 */
public class Config extends FileScanListener {

    public enum Item {
        PLUGIN_TIMEOUT_MILLIS("plugin.timeout.millis", "100"),
        HOOKS_IGNORE("hooks.ignore", ""),
        BLOCK_URL("block.url", "https://rasp.baidu.com/blocked"),
        READ_FILE_EXTENSION_REGEX("readfile.extension.regex", "^(gz|7z|xz|tar|rar|zip|sql|db)$"),
        INJECT_URL_PREFIX("inject.urlprefix", ""),
        BODY_MAX_BYTES("body.maxbytes", "4096"),
        LOG_MAX_STACK("log.maxstack", "20"),
        REFLECTION_MAX_STACK("reflection.maxstack", "100"),
        SECURITY_ENFORCE_POLICY("security.enforce_policy", "false"),
        OGNL_EXPRESSION_MIN_LENGTH("ognl.expression.minlength", "30"),
        SQL_SLOW_QUERY_MIN_ROWS("sql.slowquery.min_rows", "500"),
        REFLECTION_MONITOR("reflection.monitor",
                "java.lang.Runtime.getRuntime,java.lang.Runtime.exec,java.lang.ProcessBuilder.start");

        Item(String key, String defaultValue) {
            this.key = key;
            this.defaultValue = defaultValue;
        }

        String key;
        String defaultValue;

        @Override
        public String toString() {
            return key;
        }
    }

    public static final int REFLECTION_STACK_START_INDEX = 0;
    public static final Logger LOGGER = Logger.getLogger(Config.class.getName());
    private static String baseDirectory;

    private int reflectionMaxStack;
    private long pluginTimeout;
    private long bodyMaxBytes;
    private int sqlSlowQueryMinCount;
    private String[] ignoreHooks;
    private boolean enforcePolicy;
    private String[] reflectionMonitorMethod;
    private int logMaxStackSize;
    private String readFileExtensionRegex;
    private String blockUrl;
    private String injectUrlPrefix;
    private int ognlMinLength;


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
        CustomResponseHtml.load(baseDirectory);
        try {
            FileScanMonitor.addMonitor(
                    baseDirectory, ConfigHolder.instance);
        } catch (JNotifyException e) {
            throw new ConfigLoadException("add listener on " + baseDirectory + " failed because:" + e.getMessage());
        }
        LOGGER.info("baseDirectory: " + baseDirectory);
    }

    /**
     * 构造函数，初始化全局配置
     */
    private Config() {
        FileInputStream input = null;
        Properties properties = new Properties();
        try {
            input = new FileInputStream(new File(baseDirectory, "conf" + File.separator + "rasp.properties"));
            properties.load(input);
            input.close();
        } catch (FileNotFoundException e) {
            handleException("Could not find rasp.properties, using default settings: " + e.getMessage(), e);
        } catch (IOException e) {
            handleException("cannot load properties file: " + e.getMessage(), e);
        }
        for (Item item : Item.values()) {
            setConfigFromProperties(item, properties);
        }
        for (int i = 0; i < this.ignoreHooks.length; i++) {
            this.ignoreHooks[i] = this.ignoreHooks[i].trim();
        }
    }

    private void setConfigFromProperties(Item item, Properties properties) {
        String key = item.key;
        String value = properties.getProperty(item.key, item.defaultValue);
        setConfig(key, value);
        LOGGER.info(item.key + ": " + value);
    }

    private void handleException(String message, Exception e) {
        LOGGER.warn(message);
        System.out.println(message);
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

    /**
     * 获取自定义插入 html 页面的 js 脚本
     *
     * @return js脚本内容
     */
    public String getCustomResponseScript() {
        return CustomResponseHtml.getInstance() != null ? CustomResponseHtml.getInstance().getContent() : null;
    }

    @Override
    public void onDirectoryCreate(File file) {
        reloadCustomScript(file);
    }

    @Override
    public void onDirectoryDelete(File file) {
        reloadCustomScript(file);
    }

    @Override
    public void onFileCreate(File file) {
        // ignore
    }

    @Override
    public void onFileChange(File file) {
        // ignore
    }

    @Override
    public void onFileDelete(File file) {
        // ignore
    }

    private void reloadCustomScript(File assetsDir) {
        try {
            if (assetsDir.getName().equals(CustomResponseHtml.CUSTOM_RESPONSE_BASE_DIR)) {
                CustomResponseHtml.load(baseDirectory);
            }
        } catch (Exception e) {
            LOGGER.warn(e.getMessage());
        }
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
     * @param pluginTimeout 超时时间
     */
    public synchronized void setPluginTimeout(String pluginTimeout) {
        this.pluginTimeout = Long.parseLong(pluginTimeout);
        if (this.pluginTimeout < 0) {
            this.pluginTimeout = 0;
        }
    }

    /**
     * 设置需要插入自定义html的页面path前缀
     *
     * @return 页面path前缀
     */
    public synchronized String getInjectUrlPrefix() {
        return injectUrlPrefix;
    }

    /**
     * 获取需要插入自定义html的页面path前缀
     *
     * @param injectUrlPrefix 页面path前缀
     */
    public synchronized void setInjectUrlPrefix(String injectUrlPrefix) {
        this.injectUrlPrefix = injectUrlPrefix;
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
        if (this.bodyMaxBytes < 0) {
            this.bodyMaxBytes = 0;
        }
    }

    public synchronized int getSqlSlowQueryMinCount() {
        return sqlSlowQueryMinCount;
    }

    public synchronized void setSqlSlowQueryMinCount(String sqlSlowQueryMinCount) {
        this.sqlSlowQueryMinCount = Integer.parseInt(sqlSlowQueryMinCount);
        if (this.sqlSlowQueryMinCount < 0) {
            this.sqlSlowQueryMinCount = 0;
        }
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
        if (this.reflectionMaxStack < 0) {
            this.reflectionMaxStack = 0;
        }
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
        this.blockUrl = StringUtils.isEmpty(blockUrl) ? Item.BLOCK_URL.defaultValue : blockUrl;
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
        if (this.logMaxStackSize < 0) {
            this.logMaxStackSize = 0;
        }
    }

    /**
     * 获取允许传入插件的ognl表达式的最短长度
     *
     * @return ognl表达式最短长度
     */
    public synchronized int getOgnlMinLength() {
        return ognlMinLength;
    }

    /**
     * 配置允许传入插件的ognl表达式的最短长度
     *
     * @param ognlMinLength ognl表达式最短长度
     */
    public synchronized void setOgnlMinLength(String ognlMinLength) {
        this.ognlMinLength = Integer.parseInt(ognlMinLength);
        if (this.ognlMinLength < 0) {
            this.ognlMinLength = 0;
        }
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

    public boolean setConfig(String key, String value) {
        try {
            if (Item.BLOCK_URL.key.equals(key)) {
                setBlockUrl(value);
            } else if (Item.BODY_MAX_BYTES.key.equals(key)) {
                setBodyMaxBytes(value);
            } else if (Item.HOOKS_IGNORE.key.equals(key)) {
                setIgnoreHooks(value);
            } else if (Item.INJECT_URL_PREFIX.key.equals(key)) {
                setInjectUrlPrefix(value);
            } else if (Item.LOG_MAX_STACK.key.equals(key)) {
                setLogMaxStackSize(value);
            } else if (Item.OGNL_EXPRESSION_MIN_LENGTH.key.equals(key)) {
                setOgnlMinLength(value);
            } else if (Item.PLUGIN_TIMEOUT_MILLIS.key.equals(key)) {
                setPluginTimeout(value);
            } else if (Item.READ_FILE_EXTENSION_REGEX.key.equals(key)) {
                setReadFileExtensionRegex(value);
            } else if (Item.REFLECTION_MAX_STACK.key.equals(key)) {
                setReflectionMaxStack(value);
            } else if (Item.SECURITY_ENFORCE_POLICY.key.equals((key))) {
                setEnforcePolicy(value);
            } else if (Item.SQL_SLOW_QUERY_MIN_ROWS.key.equals(key)) {
                setSqlSlowQueryMinCount(value);
            } else if (Item.REFLECTION_MONITOR.key.equals(key)) {
                setReflectionMonitorMethod(value);
            }
        } catch (Exception e) {
            LOGGER.info(e.getMessage());
            return false;
        }
        return true;
    }
}
