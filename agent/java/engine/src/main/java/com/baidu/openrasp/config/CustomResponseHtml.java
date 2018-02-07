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

package com.baidu.openrasp.config;

import com.baidu.openrasp.exception.ConfigLoadException;
import com.baidu.openrasp.tool.filemonitor.FileScanListener;
import com.baidu.openrasp.tool.filemonitor.FileScanMonitor;
import com.fuxi.javaagent.contentobjects.jnotify.JNotifyException;
import org.apache.commons.io.FileUtils;

import java.io.File;
import java.io.IOException;

/**
 * Created by tyy on 9/21/17.
 *
 * 自定义向返回页面中插入的内容
 */
public class CustomResponseHtml extends FileScanListener {

    public static final String CUSTOM_RESPONSE_BASE_DIR = "assets";

    private static final String CUSTOM_RESPONSE_DEFAULT_NAME = "inject.html";

    private static CustomResponseHtml instance = new CustomResponseHtml("");

    private static Integer watchId = null;

    private String content;

    /**
     * 加载自定义要插入 html 页面的 js 检测脚本脚本
     * 监控自定义文件的更新
     *
     * @param basePath 安装目录
     */
    public static synchronized void load(String basePath) {
        File file = new File(basePath + File.separator + CUSTOM_RESPONSE_BASE_DIR +
                File.separator + CUSTOM_RESPONSE_DEFAULT_NAME);
        try {
            if (file.exists()) {
                instance.setContent(FileUtils.readFileToString(file));
            } else {
                instance.setContent("");
            }
            if (file.getParentFile().exists()) {
                if (watchId != null) {
                    FileScanMonitor.removeMonitor(watchId);
                }
                watchId = FileScanMonitor.addMonitor(
                        file.getParent(), instance);
            }
        } catch (JNotifyException e) {
            throw new ConfigLoadException("add listener on " + file.getAbsolutePath() + " failed because:" + e.getMessage());
        } catch (IOException e) {
            throw new ConfigLoadException("Fail to extract " + CUSTOM_RESPONSE_DEFAULT_NAME
                    + ", because of: " + e.getMessage());
        } catch (Exception e) {
            e.printStackTrace();
            throw new ConfigLoadException("Fail to init custom html " + CUSTOM_RESPONSE_DEFAULT_NAME
                    + ", because of: " + e.getMessage());
        }
    }

    /**
     * constructor
     *
     * @param content 自定义页面内容
     */
    private CustomResponseHtml(String content) {
        File assetsDir = new File(Config.getConfig().getBaseDirectory() + File.separator
                + CustomResponseHtml.CUSTOM_RESPONSE_BASE_DIR);
        if (!assetsDir.exists()) {
            assetsDir.mkdir();
        }
        this.content = content;
    }

    /**
     * 获取单例
     *
     * @return 单例
     */
    public static CustomResponseHtml getInstance() {
        return instance;
    }

    /**
     * 获取自定义脚本内容
     *
     * @return 脚本内容
     */
    public synchronized String getContent() {
        return content;
    }

    /**
     * 设置自定义js脚本
     *
     * @param content js脚本内容
     */
    public synchronized void setContent(String content) {
        this.content = content;
    }

    /**
     * 通过文件设置自定义js脚本
     * 如果文件存在设置为文件内容,如果文件不存在脚本内容设置为空字符串
     *
     * @param file
     */
    public void updateContentFormFile(File file) {
        if (file.getName().equals(CUSTOM_RESPONSE_DEFAULT_NAME)) {
            if (file.exists()) {
                try {
                    setContent(FileUtils.readFileToString(file));
                } catch (IOException e) {
                    Config.LOGGER.warn(file.getAbsoluteFile() + " update fail because:" + e.getMessage());
                }
            } else {
                setContent("");
            }
        }
    }

    @Override
    public void onFileCreate(File file) {
        instance.updateContentFormFile(file);
    }

    @Override
    public void onFileChange(File file) {
        instance.updateContentFormFile(file);
    }

    @Override
    public void onFileDelete(File file) {
        instance.updateContentFormFile(file);
    }
}
