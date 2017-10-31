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

import com.fuxi.javaagent.Agent;
import com.fuxi.javaagent.exception.ConfigLoadException;
import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.List;

/**
 * Created by tyy on 9/21/17.
 * 自定义向返回页面中插入的内容
 */
public class CustomResponseScript {

    private static final String CUSTOM_REPONSE_BASE_DIR = "assets";

    private static final String CUSTOM_REPONSE_DEFAULT_NAME = "inject.html";

    private static CustomResponseScript instance;

    private String content;

    /**
     * 加载自定义拦截返回内容
     *
     * @param basePath 安装目录
     */
    public static synchronized void load(String basePath) {
        try {
            File file = new File(basePath + File.separator + CUSTOM_REPONSE_BASE_DIR +
                    File.separator + CUSTOM_REPONSE_DEFAULT_NAME);
            if (!file.exists()) {
                return;
            }
            instance = new CustomResponseScript(FileUtils.readFileToString(file));
        } catch (IOException e) {
            throw new ConfigLoadException("Fail to extract " + CUSTOM_REPONSE_DEFAULT_NAME
                    + ", because of: " + e.getMessage());
        }
    }

    /**
     * constructor
     *
     * @param content 自定义页面内容
     */
    private CustomResponseScript(String content) {
        this.content = content;
    }

    /**
     * 获取单例
     *
     * @return
     */
    public static CustomResponseScript getInstance() {
        return instance;
    }

    public String getContent() {
        return content;
    }

    public void setContent(String content) {
        this.content = content;
    }

}
