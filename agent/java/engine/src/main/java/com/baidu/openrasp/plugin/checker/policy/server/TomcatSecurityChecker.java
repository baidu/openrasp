/*
 * Copyright 2017-2019 Baidu Inc.
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
import com.baidu.openrasp.plugin.info.SecurityPolicyInfo.Type;
import com.baidu.openrasp.tool.model.ApplicationModel;
import org.apache.commons.lang3.StringUtils;
import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import java.io.File;
import java.util.*;

/**
 * Created by tyy on 8/8/17.
 * 检测tomcat安全规范的工具类
 */
public class TomcatSecurityChecker extends ServerPolicyChecker {
    public TomcatSecurityChecker() {
        super();
    }

    public TomcatSecurityChecker(boolean canBlock) {
        super(canBlock);
    }

    private static final String TOMCAT_CHECK_ERROR_LOG_CHANNEL = "tomcat_security_check_error";
    private static final String HTTP_ONLY_ATTRIBUTE_NAME = "useHttpOnly";
    private static final String[] TOMCAT_MANAGER_ROLES = new String[]{"admin-gui", "manager-gui", "manager", "admin"};
    private static final String[] WEAK_WORDS = new String[]{"both", "tomcat", "admin", "manager", "123456", "root"};
    private static final String[] DEFAULT_APP_DIRS = new String[]{"manager", "host-manager", "docs"};
    private static final Logger LOGGER = Logger.getLogger(HookHandler.class.getName());

    @Override
    public void checkServer(CheckParameter checkParameter, List<EventInfo> infos) {
        if ("tomcat".equals(ApplicationModel.getServerName())) {
            String tomcatBaseDir = System.getProperty("catalina.base");
            try {
                if (tomcatBaseDir != null) {
                    checkHttpOnlyIsOpen(tomcatBaseDir, infos);
                    checkManagerPassword(tomcatBaseDir, infos);
                    checkDirectoryListing(tomcatBaseDir, infos);
                    checkDefaultApp(tomcatBaseDir, infos);
                    System.out.println("[OpenRASP] Tomcat security baseline - inspection completed");
                } else {
                    String message = getFormattedMessage(TOMCAT_CHECK_ERROR_LOG_CHANNEL,
                            "Unable to locate tomcat base directory: failed to read system property \"catalina.base\"");
                    int errorCode = ErrorType.PLUGIN_ERROR.getCode();
                    LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode));
                }
            } catch (Exception e) {
                handleException(e);
            }
        }
    }

    /**
     * 检测cookie的HttpOnly是否开启
     */
    private void checkHttpOnlyIsOpen(String tomcatBaseDir, List<EventInfo> infos) {
        File contextFile = new File(tomcatBaseDir + File.separator + "conf/context.xml");
        if (!contextFile.exists()) {
            String message = getFormattedMessage(TOMCAT_CHECK_ERROR_LOG_CHANNEL,
                    "Unable to load conf/context.xml: no such file");
            int errorCode = ErrorType.PLUGIN_ERROR.getCode();
            LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode));
            return;
        }

        if (!contextFile.canRead()) {
            String message = getFormattedMessage(TOMCAT_CHECK_ERROR_LOG_CHANNEL,
                    "Unable to load conf/context.xml: file is not readable");
            int errorCode = ErrorType.PLUGIN_ERROR.getCode();
            LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode));
            return;
        }

        Element contextElement = getXmlFileRootElement(contextFile);
        if (contextElement != null) {
            String httpOnly = contextElement.getAttribute(HTTP_ONLY_ATTRIBUTE_NAME);

            boolean isHttpOnly = true;
            String serverVersion = ApplicationModel.getVersion();
            if (httpOnly != null && httpOnly.equals("false")) {
                isHttpOnly = false;
            } else if (!StringUtils.isEmpty(serverVersion) && serverVersion.charAt(0) < '7' && httpOnly == null) {
                isHttpOnly = false;
            }

            if (!isHttpOnly) {
                Map<String, Object> params = new HashMap<String, Object>();
                params.put("config_file", contextFile.getAbsolutePath());
                infos.add(new SecurityPolicyInfo(Type.COOKIE_HTTP_ONLY,
                        "Tomcat security baseline - httpOnly should be enabled in " + contextFile.getAbsolutePath(), true, params));
            }
        }
    }

    /**
     * 检测tomcat后台管理员角色密码是否安全
     */
    private void checkManagerPassword(String tomcatBaseDir, List<EventInfo> infos) {
        File userFile = new File(tomcatBaseDir + File.separator + "conf/tomcat-users.xml");
        if (!(userFile.exists() && userFile.canRead())) {
            String message = getFormattedMessage(TOMCAT_CHECK_ERROR_LOG_CHANNEL,
                    "can not load file conf/tomcat-users.xml: no such file or file is not readable");
            int errorCode = ErrorType.PLUGIN_ERROR.getCode();
            LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode));
            return;
        }

        Element userElement = getXmlFileRootElement(userFile);
        if (userElement != null) {
            NodeList userNodeList = userElement.getElementsByTagName("user");
            if (userNodeList != null) {
                for (int i = 0; i < userNodeList.getLength(); i++) {
                    Node userNode = userNodeList.item(i);
                    if (userNode.getNodeType() == Node.ELEMENT_NODE) {
                        Element user = (Element) userNode;
                        String rolesAttribute = user.getAttribute("roles");
                        String[] roles = rolesAttribute == null ? null : rolesAttribute.split(",");
                        if (roles != null && roles.length > 0) {
                            List<String> managerList = Arrays.asList(TOMCAT_MANAGER_ROLES);
                            for (int j = 0; j < roles.length; j++) {
                                if (managerList.contains(roles[j].trim())) {
                                    List<String> weakWords = Arrays.asList(WEAK_WORDS);
                                    String userName = user.getAttribute("username");
                                    String password = user.getAttribute("password");
                                    if (password == null || password.isEmpty()) {
                                        Map<String, Object> params = new HashMap<String, Object>();
                                        params.put("type", ApplicationModel.getServerName());
                                        params.put("username", userName);
                                        params.put("password", password);
                                        infos.add(new SecurityPolicyInfo(Type.MANAGER_PASSWORD,
                                                "Tomcat security baseline - detected empty password in " +
                                                        userFile.getAbsolutePath() + ", username is " + userName, true, params));
                                    }
                                    if (weakWords.contains(userName) && weakWords.contains(password)) {
                                        Map<String, Object> params = new HashMap<String, Object>();
                                        params.put("type", ApplicationModel.getServerName());
                                        params.put("username", userName);
                                        params.put("password", password);
                                        infos.add(new SecurityPolicyInfo(Type.MANAGER_PASSWORD,
                                                "Tomcat security baseline - detected weak username/password combination in " + userFile.getAbsolutePath() +
                                                        ", username is " + userName, true, params));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    private void checkDirectoryListing(String tomcatBaseDir, List<EventInfo> infos) {
        File webFile = new File(tomcatBaseDir + File.separator + "conf/web.xml");
        if (!(webFile.exists() && webFile.canRead())) {
            String message = getFormattedMessage(TOMCAT_CHECK_ERROR_LOG_CHANNEL,
                    "can not load file conf/web.xml: no such file or file is not readable");
            int errorCode = ErrorType.PLUGIN_ERROR.getCode();
            LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode));
            return;
        }
        Element rootElement = getXmlFileRootElement(webFile);
        if (rootElement != null) {
            NodeList servletList = rootElement.getElementsByTagName("servlet");
            if (servletList != null) {
                for (int i = 0; i < servletList.getLength(); i++) {
                    Node node = servletList.item(i);
                    if (node.getNodeType() == Node.ELEMENT_NODE) {
                        Element servletElement = (Element) node;
                        NodeList servletClassList = servletElement.getElementsByTagName("servlet-class");
                        if (servletClassList != null) {
                            boolean isFoundDefaultClass = false;
                            for (int j = 0; j < servletClassList.getLength(); j++) {
                                String className = servletClassList.item(j).getTextContent();
                                if ("org.apache.catalina.servlets.DefaultServlet".equals(className)) {
                                    isFoundDefaultClass = true;
                                    break;
                                }
                            }
                            if (isFoundDefaultClass) {
                                if (isOpenListingDirectory(servletElement)) {
                                    Map<String, Object> params = new HashMap<String, Object>();
                                    params.put("config_file", webFile.getAbsolutePath());
                                    infos.add(new SecurityPolicyInfo(Type.DIRECTORY_LISTING,
                                            "Tomcat security baseline - detected open Directory Listing in conf/web.xml", true, params));
                                    return;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    private boolean isOpenListingDirectory(Element servletElement) {
        boolean result = false;
        NodeList initParamList = servletElement.getElementsByTagName("init-param");
        if (initParamList != null) {
            for (int z = 0; z < initParamList.getLength(); z++) {
                Node node = initParamList.item(z);
                if (node.getNodeType() == Node.ELEMENT_NODE) {
                    Element initParamElement = (Element) node;
                    NodeList paramName = initParamElement.getElementsByTagName("param-name");
                    if (paramName != null && paramName.getLength() > 0) {
                        if ("listings".equals(paramName.item(0).getTextContent())) {
                            NodeList paramValue = initParamElement.getElementsByTagName("param-value");
                            if (paramValue != null && paramValue.getLength() > 0) {
                                if ("true".equals(paramValue.item(0).getTextContent())) {
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }
        return result;
    }

    /**
     * 获取xml文档根元素
     *
     * @param xmlFile 待解析的xml文件
     * @return xml根节点
     */
    private Element getXmlFileRootElement(File xmlFile) {
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document document = builder.parse(xmlFile);
            return document.getDocumentElement();
        } catch (Exception e) {
            handleException(e);
        }
        return null;
    }


    /**
     * 检测是否删除了默认安装的app
     */
    private void checkDefaultApp(String tomcatBaseDir, List<EventInfo> infos) {
        LinkedList<String> apps = new LinkedList<String>();
        String defaultAppBaseDir = tomcatBaseDir + File.separator + "webapps";
        for (String dir : DEFAULT_APP_DIRS) {
            File file = new File(defaultAppBaseDir + File.separator + dir);
            if (file.exists() && file.isDirectory()) {
                apps.add(dir);
            }
        }

        if (!apps.isEmpty()) {
            StringBuilder message = new StringBuilder("Tomcat security baseline - did not remove the following default webapps in " + defaultAppBaseDir + ": ");
            for (String app : apps) {
                message.append(app).append(", ");
            }
            Map<String, Object> params = new HashMap<String, Object>();
            params.put("path", defaultAppBaseDir);
            params.put("apps", apps);
            infos.add(new SecurityPolicyInfo(Type.DEFAULT_APP, message.substring(0, message.length() - 2), true, params));
        }
    }

    private void handleException(Exception e) {
        String message = getFormattedMessage(TOMCAT_CHECK_ERROR_LOG_CHANNEL, e.getMessage());
        int errorCode = ErrorType.HOOK_ERROR.getCode();
        LOGGER.error(CloudUtils.getExceptionObject(message, errorCode), e);
    }

    private String getFormattedMessage(String title, String message) {
        return title + ": " + message;
    }

}
