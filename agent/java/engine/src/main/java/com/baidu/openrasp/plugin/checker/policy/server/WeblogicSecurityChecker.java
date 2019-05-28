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

package com.baidu.openrasp.plugin.checker.policy.server;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.info.EventInfo;
import com.baidu.openrasp.plugin.info.SecurityPolicyInfo;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.model.ApplicationModel;
import org.apache.log4j.Logger;

import java.io.*;
import java.util.*;

/**
 * @description: weblogic基线检查
 * @author: anyang
 * @create: 2018/09/11 11:12
 */
public class WeblogicSecurityChecker extends ServerPolicyChecker {
    private static final String WEBLOGIC_CHECK_ERROR_LOG_CHANNEL = "weblogic_security_check_error";
    private static final String[] WEAK_WORDS = new String[]{"weblogic", "weblogic1", "weblogic123", "admin", "123456", "welcome1"};
    private static final Logger LOGGER = Logger.getLogger(HookHandler.class.getName());

    public WeblogicSecurityChecker() {
    }

    public WeblogicSecurityChecker(boolean canBlock) {
        super(canBlock);
    }

    @Override
    public void checkServer(CheckParameter checkParameter, List<EventInfo> infos) {
        String domainPath = System.getProperty("user.dir");
        checkManagerPassword(domainPath, infos);
    }

    private void checkManagerPassword(String domainPath, List<EventInfo> infos) {
        List<String> paths = searchFiles(new File(domainPath), "boot.properties");
        if (paths.size() > 0) {
            File bootProperties = new File(paths.get(0));
            if (!(bootProperties.exists() && bootProperties.canRead())) {
                String message = WEBLOGIC_CHECK_ERROR_LOG_CHANNEL + ": can not load file " + paths.get(0);
                int errorCode = ErrorType.PLUGIN_ERROR.getCode();
                LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode));
            }
            String encryptedPassword = getProperties(bootProperties, "password");
            String decryptedPassword = decrypt(encryptedPassword, domainPath);
            List<String> checkList = Arrays.asList(WEAK_WORDS);
            if (checkList.contains(decryptedPassword)) {
                String encryptedUserName = getProperties(bootProperties, "username");
                String decryptedUserName = decrypt(encryptedUserName, domainPath);
                Map<String, Object> params = new HashMap<String, Object>();
                params.put("type", ApplicationModel.getServerName());
                params.put("username", decryptedUserName);
                params.put("password", decryptedPassword);
                params.put("config_file", paths.get(0));
                infos.add(new SecurityPolicyInfo(SecurityPolicyInfo.Type.MANAGER_PASSWORD, "Weblogic security baseline - detected weak password \"" +
                        decryptedPassword + "\" in config file: " + paths.get(0), true, params));
            }
        }
    }

    private String getProperties(File file, String keyWord) {
        Properties prop = new Properties();
        String value = null;
        try {
            InputStream InputStream = new BufferedInputStream(new FileInputStream(file));
            prop.load(InputStream);
            value = prop.getProperty(keyWord);
        } catch (Exception e) {
            String message = WEBLOGIC_CHECK_ERROR_LOG_CHANNEL + ": can not find " + keyWord;
            int errorCode = ErrorType.PLUGIN_ERROR.getCode();
            LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
        }
        return value != null ? value : "";
    }

    private String decrypt(String decrypted, String path) {
        String decryptedString = null;
        try {
            ClassLoader classLoader = ClassLoader.getSystemClassLoader();
            Object encryptionService = Reflection.invokeStaticMethod("weblogic.security.internal.SerializedSystemIni", "getEncryptionService", new Class[]{String.class}, path);
            if (encryptionService != null) {
                Object clearOrEncryptedService = classLoader.loadClass("weblogic.security.internal.encryption.ClearOrEncryptedService").getDeclaredConstructor(classLoader.loadClass("weblogic.security.internal.encryption.EncryptionService")).newInstance(encryptionService);
                decryptedString = Reflection.invokeStringMethod(clearOrEncryptedService, "decrypt", new Class[]{String.class}, decrypted);
            }
        } catch (Exception e) {
            String message = WEBLOGIC_CHECK_ERROR_LOG_CHANNEL + ": can not decrypt the encryptedString";
            int errorCode = ErrorType.PLUGIN_ERROR.getCode();
            LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
        }
        return decryptedString != null ? decryptedString : "";
    }

    private List<String> searchFiles(File folder, final String keyword) {
        List<String> result = new ArrayList<String>();
        if (folder.isFile()) {
            result.add(folder.getAbsolutePath());
        }
        File[] subFolders = folder.listFiles(new FileFilter() {
            public boolean accept(File file) {
                return file.isDirectory() || file.getName().equals(keyword);
            }
        });
        if (subFolders != null) {
            for (File file : subFolders) {
                if (file.isFile()) {
                    result.add(file.getAbsolutePath());
                } else {
                    result.addAll(searchFiles(file, keyword));
                }
            }
        }
        return result;
    }
}
