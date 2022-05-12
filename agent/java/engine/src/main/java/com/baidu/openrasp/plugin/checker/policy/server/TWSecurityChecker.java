package com.baidu.openrasp.plugin.checker.policy.server;

import com.baidu.openrasp.messaging.ErrorType;
import com.baidu.openrasp.messaging.LogTool;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.info.EventInfo;
import com.baidu.openrasp.plugin.info.SecurityPolicyInfo;
import com.baidu.openrasp.tool.model.ApplicationModel;
import org.w3c.dom.*;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import java.io.File;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * tongweb8
 */
public class TWSecurityChecker extends ServerPolicyChecker {
    private static final String TW_CHECK_ERROR_LOG_CHANNEL = "TW_security_check_error";
    private static final String APP = "app";
    private static final String APP_ID = "appId";
    private static final String HTTP_ONLY_ATTRIBUTE_NAME = "useHttpOnly";

    public TWSecurityChecker() {
        super();
    }

    public TWSecurityChecker(boolean canBlock) {
        super(canBlock);
    }

    @Override
    public void checkServer(CheckParameter checkParameter, List<EventInfo> infos) {
        if ("TW".equals(ApplicationModel.getServerName())) {
            String tongwebBaseDir = System.getProperty("tongweb.base");
            try {
                if (tongwebBaseDir != null) {
                    checkHttpOnlyIsOpen(tongwebBaseDir, infos);
                    checkDirectoryListing(tongwebBaseDir, infos);
                    System.out.println("[OpenRASP] TW security baseline - inspection completed");
                } else {
                    LogTool.warn(ErrorType.PLUGIN_ERROR, getFormattedMessage(TW_CHECK_ERROR_LOG_CHANNEL,
                            "Unable to locate TW base directory: failed to read system property \"tongweb.base\""));
                }
            } catch (Exception e) {
                handleException(e);
            }
        }
    }

    /**
     * 检测cookie的HttpOnly是否开启
     */
    private void checkHttpOnlyIsOpen(String tongWebBaseDir, List<EventInfo> infos) {
        File contextFile = new File(tongWebBaseDir + File.separator + "conf/applications.xml");
        if (!contextFile.exists()) {
            LogTool.warn(ErrorType.PLUGIN_ERROR, getFormattedMessage(TW_CHECK_ERROR_LOG_CHANNEL,
                    "Unable to load conf/applications.xml: no such file, ignored"));
            return;
        }

        if (!contextFile.canRead()) {
            LogTool.warn(ErrorType.PLUGIN_ERROR, getFormattedMessage(TW_CHECK_ERROR_LOG_CHANNEL,
                    "Unable to load conf/applications.xml: file is not readable, ignored"));
            return;
        }

        Document document = getXmlFileRootElement(contextFile);
        if (document != null) {
            NodeList apps = document.getElementsByTagName(APP);
            int length = apps.getLength();
            for (int i = 0; i < length; i++) {
                Node item = apps.item(i);
                NamedNodeMap attributes = item.getAttributes();
                Node httpOnlyNamedItem = attributes.getNamedItem(HTTP_ONLY_ATTRIBUTE_NAME);
                String httpOnly = httpOnlyNamedItem.getNodeValue();
                boolean isHttpOnly = true;
                if (httpOnly != null && httpOnly.equals("false")) {
                    isHttpOnly = false;
                }
                if (!isHttpOnly) {
                    Node appIdNamedItem = attributes.getNamedItem(APP_ID);
                    Map<String, Object> params = new HashMap<String, Object>();
                    params.put("config_file", contextFile.getAbsolutePath());
                    params.put("appId", appIdNamedItem.getNodeValue());
                    infos.add(new SecurityPolicyInfo(SecurityPolicyInfo.Type.COOKIE_HTTP_ONLY,
                            "TW security baseline - httpOnly should be enabled in " + contextFile.getAbsolutePath(), true, params));
                }
            }
        }
    }


    private void checkDirectoryListing(String tongWebBaseDir, List<EventInfo> infos) {
        File webFile = new File(tongWebBaseDir + File.separator + "conf/default-web.xml");
        if (!(webFile.exists() && webFile.canRead())) {
            LogTool.warn(ErrorType.PLUGIN_ERROR, getFormattedMessage(TW_CHECK_ERROR_LOG_CHANNEL,
                    "can not load file conf/default-web.xml: no such file or file is not readable"));
            return;
        }
        Document document = getXmlFileRootElement(webFile);
        if (document != null) {
            NodeList servletList = document.getElementsByTagName("servlet");
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
                                if ("com.tongweb.server.servlets.DefaultServlet".equals(className)) {
                                    isFoundDefaultClass = true;
                                    break;
                                }
                            }
                            if (isFoundDefaultClass) {
                                if (isOpenListingDirectory(servletElement)) {
                                    Map<String, Object> params = new HashMap<String, Object>();
                                    params.put("config_file", webFile.getAbsolutePath());
                                    infos.add(new SecurityPolicyInfo(SecurityPolicyInfo.Type.DIRECTORY_LISTING,
                                            "TW security baseline - detected open Directory Listing in conf/default-web.xml", true, params));
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
    private Document getXmlFileRootElement(File xmlFile) {
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            return builder.parse(xmlFile);
        } catch (Exception e) {
            handleException(e);
        }
        return null;
    }


    private void handleException(Exception e) {
        LogTool.warn(ErrorType.PLUGIN_ERROR, getFormattedMessage(TW_CHECK_ERROR_LOG_CHANNEL, e.getMessage()), e);
    }

    private String getFormattedMessage(String title, String message) {
        return title + ": " + message;
    }
}
