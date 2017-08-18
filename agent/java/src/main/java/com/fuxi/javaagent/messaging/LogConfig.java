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

package com.fuxi.javaagent.messaging;

import com.fuxi.javaagent.Agent;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.IOException;


/**
 * Created by lxk on 17-4-10.
 * 用于导出日志配置
 */
public class LogConfig {

    static final String SEPARATOR;
    static final String DELIMITER;
    static final String CONFIGFOLDER;
    static final String CONFIGFILE;
    static final String BUMMYTARGETPATH;

    static {
        DELIMITER = System.getProperty("line.separator");
        SEPARATOR = System.getProperty("file.separator");
        CONFIGFOLDER = "conf";
        CONFIGFILE = "rasp-log4j.xml";
        BUMMYTARGETPATH = "[[TARGET PATH]]";
    }

    /**
     *
     * @param raspRootDirectory rasp根目录
     */
    public static boolean completeLogConfig(String raspRootDirectory) {
        String logConfigFile = raspRootDirectory + SEPARATOR + CONFIGFOLDER + SEPARATOR + CONFIGFILE;
        if (isLogConfigFileExist(logConfigFile)) {
            return true;
        }
        return extractLogConfigFile(raspRootDirectory);
    }

    /**
     *
     * @param logConfigFile 日志配置文件
     * @return  配置文件已存在返回true,否则false
     */
    private static boolean isLogConfigFileExist(String logConfigFile) {
        File configFile = new File(logConfigFile);
        if (configFile.exists() && configFile.isFile()) {
            System.out.println("Log config file already exist!");
            return true;
        }
        return false;
    }

    /**
     *
     * @param raspRootDirectory rasp根目录
     * @return 导出文件成功返回true,否则false
     */
    private static boolean extractLogConfigFile(String raspRootDirectory) {
        InputStream inputStream = null;
        FileWriter fileWriter = null;
        BufferedReader bufferedReader = null;
        BufferedWriter bufferedWriter = null;
        try {
            new File(raspRootDirectory + SEPARATOR + CONFIGFOLDER).mkdirs();
            String raspRootAbsPath = new File(raspRootDirectory).getAbsolutePath();
            if (SEPARATOR.equals("\\")) {
                raspRootAbsPath = raspRootDirectory.replaceAll("\\\\", "/");
            }
            inputStream = Agent.class.getResourceAsStream("/" + CONFIGFILE + ".default");
            fileWriter = new FileWriter(raspRootDirectory + SEPARATOR + CONFIGFOLDER + SEPARATOR + CONFIGFILE);
            bufferedReader = new BufferedReader(new InputStreamReader(inputStream));
            bufferedWriter = new BufferedWriter(fileWriter);
            String lineContent = null;
            while ((lineContent = bufferedReader.readLine()) != null) {
                String trimedLine = lineContent.trim();
                if (!trimedLine.startsWith("<!--") && trimedLine.startsWith("<param name=\"File\"")
                        && lineContent.contains(BUMMYTARGETPATH)) {
                    lineContent = lineContent.replace(BUMMYTARGETPATH, raspRootAbsPath);
                }
                bufferedWriter.write(lineContent);
                bufferedWriter.newLine();
            }
            bufferedWriter.close();
            bufferedReader.close();
            fileWriter.close();
            inputStream.close();
        } catch (Throwable throwable) {
            try {
                if (bufferedWriter != null) {
                    bufferedWriter.close();
                }
            } catch (IOException ebw) {
                ebw.printStackTrace();
            }
            try {
                if (bufferedReader != null) {
                    bufferedWriter.close();
                }
            } catch (IOException ebr) {
                ebr.printStackTrace();
            }
            try {
                if (fileWriter != null) {
                    fileWriter.close();
                }
            } catch (IOException efw) {
                efw.printStackTrace();
            }
            try {
                if (inputStream != null) {
                    inputStream.close();
                }
            } catch (IOException eis) {
                eis.printStackTrace();
            }
            throwable.printStackTrace();
            System.out.println("Fail to extract " + CONFIGFILE + ", because of: " + throwable.getMessage());
            return false;
        }
        return true;
    }
}
