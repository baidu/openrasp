/*
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
import org.apache.commons.io.FileUtils;

import java.io.File;
import java.io.IOException;

/**
 * Created by tyy on 9/21/17.
 *
 * 自定义向返回页面中插入的内容
 */
public class CustomResponseScript extends FileScanListener {

    public static final String CUSTOM_RESPONSE_BASE_DIR = "assets";

    private static final String CUSTOM_RESPONSE_DEFAULT_NAME = "inject.js";

    private static CustomResponseScript instance = new CustomResponseScript("");

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
            throw new ConfigLoadException("Fail to init custom script " + CUSTOM_RESPONSE_DEFAULT_NAME
                    + ", because of: " + e.getMessage());
        }
    }

    /**
     * constructor
     *
     * @param content 自定义页面内容
     */
    private CustomResponseScript(String content) {
        File assetsDir = new File(Config.getConfig().getBaseDirectory() + File.separator
                + CustomResponseScript.CUSTOM_RESPONSE_BASE_DIR);
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
    public static CustomResponseScript getInstance() {
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
