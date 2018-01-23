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

package com.baidu.openrasp.messaging;

import com.baidu.openrasp.Agent;
import com.baidu.openrasp.exception.ConfigLoadException;

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
    public static void completeLogConfig(String raspRootDirectory) {
        String logConfigFile = raspRootDirectory + SEPARATOR + CONFIGFOLDER + SEPARATOR + CONFIGFILE;
        if (!isLogConfigFileExist(logConfigFile)) {
            extractLogConfigFile(raspRootDirectory);
        }
    }

    /**
     *
     * @param logConfigFile 日志配置文件
     * @return  配置文件已存在返回true,否则false
     */
    private static boolean isLogConfigFileExist(String logConfigFile) {
        File configFile = new File(logConfigFile);
        if (configFile.exists() && configFile.isFile()) {
            System.out.println("[OpenRASP] Log config file already exists, continuing ...");
            return true;
        }
        return false;
    }

    /**
     *
     * @param raspRootDirectory rasp根目录
     * @return 导出文件成功返回true,否则false
     */
    private static void extractLogConfigFile(String raspRootDirectory) {
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
            throw new ConfigLoadException("[OpenRASP] Unable to extract log4j config file: " + CONFIGFILE + ", error: " + throwable.getMessage());
        }
    }
}
