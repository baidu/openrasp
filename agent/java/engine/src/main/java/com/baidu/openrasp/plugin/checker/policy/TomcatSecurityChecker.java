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

package com.baidu.openrasp.plugin.checker.policy;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.info.EventInfo;
import com.baidu.openrasp.plugin.info.SecurityPolicyInfo;
import com.baidu.openrasp.plugin.info.SecurityPolicyInfo.Type;
import com.google.gson.JsonObject;
import org.apache.commons.lang3.StringUtils;
import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import java.io.File;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;

/**
 * Created by tyy on 8/8/17.
 * 检测tomcat安全规范的工具类
 */
public class TomcatSecurityChecker extends PolicyChecker {

    private static final String TOMCAT_CHECK_ERROR_LOG_CHANNEL = "tomcat_security_check_error";
    private static final String HTTP_ONLY_ATTRIBUTE_NAME = "useHttpOnly";
    private static final String WINDOWS_ADMIN_GROUP_ID = "S-1-5-32-544";
    private static final String[] TOMCAT_MANAGER_ROLES = new String[]{"admin-gui", "manager-gui", "manager", "admin"};
    private static final String[] WEAK_WORDS = new String[]{"both", "tomcat", "admin", "manager", "123456", "root"};
    private static final String[] DEFAULT_APP_DIRS = new String[]{"ROOT", "manager", "host-manager", "docs"};
    private static final Logger LOGGER = Logger.getLogger(HookHandler.class.getName());

    @Override
    public List<EventInfo> checkParam(CheckParameter checkParameter) {
        String tomcatBaseDir = System.getProperty("catalina.base");
        List<EventInfo> infos = new LinkedList<EventInfo>();
        try {
            String serverType = SecurityPolicyInfo.getCatalinaServerType();
            if (tomcatBaseDir != null && serverType != null) {
                checkStartUser(infos);
                if (serverType.equalsIgnoreCase("tomcat")) {
                    checkHttpOnlyIsOpen(tomcatBaseDir, infos);
                    checkManagerPassword(tomcatBaseDir, infos);
                    checkDirectoryListing(tomcatBaseDir, infos);
                    checkDefaultApp(tomcatBaseDir, infos);

                    System.out.println("[OpenRASP] Tomcat security baseline - inspection completed");
                }
            } else {
                LOGGER.warn(getFormattedMessage(TOMCAT_CHECK_ERROR_LOG_CHANNEL,
                        "can not find tomcat base directory"));
            }
        } catch (Exception e) {
            handleException(e);
        }

        return infos;
    }

    /**
     * 检测cookie的HttpOnly是否开启
     */
    private void checkHttpOnlyIsOpen(String tomcatBaseDir, List<EventInfo> infos) {
        File contextFile = new File(tomcatBaseDir + File.separator + "conf/context.xml");
        if (!(contextFile.exists() && contextFile.canRead())) {
            LOGGER.warn(getFormattedMessage(TOMCAT_CHECK_ERROR_LOG_CHANNEL,
                    "can not load file conf/context.xml"));
            return;
        }

        Element contextElement = getXmlFileRootElement(contextFile);
        if (contextElement != null) {
            String httpOnly = contextElement.getAttribute(HTTP_ONLY_ATTRIBUTE_NAME);

            boolean isHttpOnly = true;
            String serverVersion = SecurityPolicyInfo.getCatalinaServerVersion();
            if (httpOnly != null && httpOnly.equals("false")) {
                isHttpOnly = false;
            } else if (!StringUtils.isEmpty(serverVersion) && serverVersion.charAt(0) < '7' && httpOnly == null) {
                isHttpOnly = false;
            }

            if (!isHttpOnly) {
                infos.add(new SecurityPolicyInfo(Type.COOKIE_HTTP_ONLY, "tomcat未在conf/context.xml文件中配置全局httpOnly.", true));
            }
        }
    }

    /**
     * 检测启动用户是否为系统管理员
     */
    private void checkStartUser(List<EventInfo> infos) {
        String osName = System.getProperty("os.name").toLowerCase();
        if (osName.startsWith("linux") || osName.startsWith("mac")) {
            if ("root".equals(System.getProperty("user.name"))) {
                infos.add(new SecurityPolicyInfo(Type.START_USER, "tomcat以root权限启动.", true));
            }
        } else if (osName.startsWith("windows")) {
            try {
                Class<?> ntSystemClass = Class.forName("com.sun.security.auth.module.NTSystem");
                Object ntSystemObject = ntSystemClass.newInstance();
                String[] userGroups = (String[]) ntSystemClass.getMethod("getGroupIDs").invoke(ntSystemObject);
                if (userGroups != null) {
                    for (String group : userGroups) {
                        if (group.equals(WINDOWS_ADMIN_GROUP_ID)) {
                            infos.add(new SecurityPolicyInfo(Type.START_USER, "服务器以管理员权限启动.", true));
                        }
                    }
                }
            } catch (Exception e) {
                handleException(e);
            }
        }
    }

    /**
     * 检测tomcat后台管理员角色密码是否安全
     */
    private void checkManagerPassword(String tomcatBaseDir, List<EventInfo> infos) {
        File userFile = new File(tomcatBaseDir + File.separator + "conf/tomcat-users.xml");
        if (!(userFile.exists() && userFile.canRead())) {
            LOGGER.warn(getFormattedMessage(TOMCAT_CHECK_ERROR_LOG_CHANNEL,
                    "can not load file conf/tomcat-users.xml"));
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
                                    if (weakWords.contains(userName) && weakWords.contains(password)) {
                                        infos.add(new SecurityPolicyInfo(Type.MANAGER_PASSWORD, "tomcat后台管理角色存在弱用户名和弱密码.", true));
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
            LOGGER.warn(getFormattedMessage(TOMCAT_CHECK_ERROR_LOG_CHANNEL,
                    "can not load file conf/web.xml"));
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
                                    infos.add(new SecurityPolicyInfo(Type.DIRECTORY_LISTING,
                                            "tomcat 开启了 DefaultServlet 的 Directory Listing 功能", true));
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
        for (String dir : DEFAULT_APP_DIRS) {
            File file = new File(tomcatBaseDir + File.separator + "webapps" + File.separator + dir);
            if (file.exists() && file.isDirectory()) {
                apps.add(dir);
            }
        }

        if (!apps.isEmpty()) {
            StringBuilder message = new StringBuilder("tomcat 默认安装的webapps没有卸载: ");
            for (String app : apps) {
                message.append(app).append(", ");
            }
            infos.add(new SecurityPolicyInfo(Type.DEFAULT_APP, message.substring(0, message.length() - 2), true));
        }
    }

    private void handleException(Exception e) {
        e.printStackTrace();
        LOGGER.error(getFormattedMessage(TOMCAT_CHECK_ERROR_LOG_CHANNEL, e.getMessage()), e);
    }

    private String getFormattedMessage(String title, String message) {
        return title + ": " + message;
    }

}
