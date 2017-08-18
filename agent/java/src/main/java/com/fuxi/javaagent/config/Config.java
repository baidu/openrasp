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
import java.util.Arrays;
import java.util.Properties;


/**
 * Created by tyy on 3/27/17.
 * <p>
 * 项目配置类，通过解析conf/rasp.property文件来加载配置
 * 若没有找到配置文件使用默认值
 */
public class Config {
    private static final String DEFAULT_V8TIMEOUT = "100";
    private static final String DEFAULT_POOLSIZE = "0";
    private static final String DEFAULT_BODYSIZE = "4096";
    private static final String DEFAULT_IGNORE = "";

    private static final Logger LOGGER = Logger.getLogger(Config.class.getName());
    private static String baseDirectory;

    private int v8ThreadPoolSize;
    private long v8Timeout;
    private long bodyMaxBytes;
    private String[] ignoreHooks;

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
        baseDirectory = new File(path).getParent();
    }

    /**
     * 构造函数，初始化全局配置
     */
    private Config() {
        FileInputStream input = null;

        this.v8ThreadPoolSize = Integer.parseInt(DEFAULT_POOLSIZE);
        this.v8Timeout = Long.parseLong(DEFAULT_V8TIMEOUT);
        this.bodyMaxBytes = Long.parseLong(DEFAULT_BODYSIZE);
        this.ignoreHooks = new String[]{};

        try {
            input = new FileInputStream(new File(baseDirectory, "conf" + File.separator + "rasp.properties"));
            Properties properties = new Properties();
            properties.load(input);

            this.v8ThreadPoolSize = Integer.parseInt(properties.getProperty("v8.threadpool.size", DEFAULT_POOLSIZE));
            this.v8Timeout = Long.parseLong(properties.getProperty("v8.timeout.millis", DEFAULT_V8TIMEOUT));
            this.bodyMaxBytes = Long.parseLong(properties.getProperty("body.maxbytes", DEFAULT_BODYSIZE));
            this.ignoreHooks = properties.getProperty("hooks.ignore", DEFAULT_IGNORE).split(",");
        } catch (FileNotFoundException e) {
            LOGGER.warn("cannot read config file: " + e.getMessage());
        } catch (IOException e) {
            LOGGER.warn("cannot load properties file: " + e.getMessage());
        } finally {
            try {
                input.close();
            } catch (Exception ignored) {
                // ignore
            }
        }

        if (this.v8ThreadPoolSize <= 0) {
            this.v8ThreadPoolSize = Runtime.getRuntime().availableProcessors() + 1;
        }
        for (int i = 0; i < this.ignoreHooks.length; i++) {
            this.ignoreHooks[i] = this.ignoreHooks[i].trim();
        }

        LOGGER.info("baseDirectory: " + baseDirectory);
        LOGGER.info("v8.threadpool.size: " + v8ThreadPoolSize);
        LOGGER.info("v8.timeout.millis: " + v8Timeout);
        LOGGER.info("hooks.ignore: " + Arrays.toString(this.ignoreHooks));
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
     * 获取V8执行超时时间
     *
     * @return 超时时间
     */
    public long getV8Timeout() {
        return v8Timeout;
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
     * 获取V8线程池容量
     *
     * @return V8线程池容量
     */
    public int getV8ThreadPoolSize() {
        return v8ThreadPoolSize;
    }

    /**
     * 保存HTTP请求体时最大保存长度
     *
     * @return 最大长度
     */
    public long getBodyMaxBytes() {
        return bodyMaxBytes;
    }

    /**
     * 需要忽略的挂钩点
     *
     * @return 需要忽略的挂钩点列表
     */
    public String[] getIgnoreHooks() {
        return this.ignoreHooks;
    }
}
