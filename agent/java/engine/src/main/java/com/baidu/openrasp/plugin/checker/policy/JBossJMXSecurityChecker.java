package com.baidu.openrasp.plugin.checker.policy;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.server.jboss.JBossStartupHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.info.EventInfo;
import com.baidu.openrasp.plugin.info.SecurityPolicyInfo;
import com.baidu.openrasp.tool.model.ApplicationModel;
import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import java.io.File;
import java.util.*;

/**
 * 　　* @Description: 检测JBoss的JMX Console配置
 * 　　* @author anyang
 * 　　* @date 2018/7/30 15:51
 */
public class JBossJMXSecurityChecker extends PolicyChecker {

    public static final String JBOSS_SECURITY_CHECK_ERROR = "jboss_security_check_error";
    private static final String SECURITY_DOMAIN = "security-domain";
    private static final String SECURITY_CONSTRAINT = "security-constraint";
    private static final String WEB_RESOURCE_COLLECTION = "web-resource-collection";
    private static final String WEB_RESOURCE_NAME = "web-resource-name";
    private static final String URL_PATTERN = "url-pattern";
    private static final String AUTH_CONSTRAINT = "auth-constraint";
    private static final String[] WEB_RESOURCE_NAME_VALUES = new String[]{"htmladaptor"};
    private static final String[] URL_PATTERN_VALUES = new String[]{"/*"};
    public static final Logger LOGGER = Logger.getLogger(HookHandler.class.getName());

    @Override
    public List<EventInfo> checkParam(CheckParameter checkParameter) {

        String jbossBaseDir = System.getProperty("jboss.home.dir");
        List<EventInfo> infos = new LinkedList<EventInfo>();
        String serverVersion = ApplicationModel.getVersion();
        if (serverVersion != null) {
            String jbossWebXmlPath = "deploy" + File.separator + "jmx-console.war" + File.separator + "WEB-INF" + File.separator + "jboss-web.xml";
            String webXmlPath = "deploy" + File.separator + "jmx-console.war" + File.separator + "WEB-INF" + File.separator + "web.xml";
            if (serverVersion.startsWith("4") || serverVersion.startsWith("5")) {
                jbossWebXmlPath = jbossBaseDir + File.separator + "server" + File.separator + "default" + File.separator + jbossWebXmlPath;
                webXmlPath = jbossBaseDir + File.separator + "server" + File.separator + "default" + File.separator + webXmlPath;
            } else if (serverVersion.startsWith("6")) {
                jbossWebXmlPath = jbossBaseDir + File.separator + "common" + File.separator + jbossWebXmlPath;
                webXmlPath = jbossBaseDir + File.separator + "common" + File.separator + webXmlPath;
            } else {

                LOGGER.error(JBOSS_SECURITY_CHECK_ERROR + " :" + "JBoss supported 4.x-7.x");
            }
            checkJBossWebXml(jbossWebXmlPath, infos);
            checkWebXml(webXmlPath, infos);
        }

        return infos;
    }


    /**
     * 检测JBoss的JBoss-web.xml是否配置security-domain
     */
    public void checkJBossWebXml(String path, List<EventInfo> infos) {
        Document root = getXMLDocument(path);
        if (root != null) {
            NodeList list = root.getElementsByTagName(SECURITY_DOMAIN);
            if (list.getLength() == 0) {
                handleError(SECURITY_DOMAIN, path, infos);
            }
        }
    }


    /**
     * 检测JBoss的web.xml是否配置security-constraint
     */
    public void checkWebXml(String path, List<EventInfo> infos) {
        Document root = getXMLDocument(path);
        if (root != null) {
            NodeList list = root.getElementsByTagName(SECURITY_CONSTRAINT);
            if (list.getLength() > 0) {
                for (int i = 0; i < list.getLength(); i++) {
                    Element element = (Element) list.item(i);
                    NodeList webResource = element.getElementsByTagName(WEB_RESOURCE_COLLECTION);
                    if (webResource.getLength() > 0) {
                        Element subElement = (Element) list.item(i);
                        checkXmlElement(subElement, WEB_RESOURCE_NAME, Arrays.asList(WEB_RESOURCE_NAME_VALUES), infos, path);
                        checkXmlElement(subElement, URL_PATTERN, Arrays.asList(URL_PATTERN_VALUES), infos, path);
                    } else {
                        handleError(WEB_RESOURCE_COLLECTION, path, infos);
                    }

                    NodeList authConstraint = element.getElementsByTagName("auth-constraint");
                    if (authConstraint.getLength() == 0) {
                        handleError(AUTH_CONSTRAINT, path, infos);
                    }
                }

            } else {
                handleError(SECURITY_CONSTRAINT, path, infos);
            }
        }
    }


    /**
     * 获取xml文件的root元素
     */
    public Document getXMLDocument(String path) {
        File file = new File(path);
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            return builder.parse(file);
        } catch (Exception e) {
            e.printStackTrace();
            LOGGER.error(JBOSS_SECURITY_CHECK_ERROR + ": " + e.getMessage(), e);
        }
        return null;
    }

    public void checkXmlElement(Element element, String key, List<String> reference, List<EventInfo> infos, String path) {
        NodeList list = element.getElementsByTagName(key);
        if (list.getLength() > 0) {
            for (int i = 0; i < list.getLength(); i++) {
                if (!reference.contains(list.item(i).getTextContent().toLowerCase())) {
                    handleError(key, path, infos);
                }
            }

        } else {
            handleError(key, path, infos);
        }
    }


    public void handleError(String tagName, String path, List<EventInfo> infos) {
        infos.add(new SecurityPolicyInfo(SecurityPolicyInfo.Type.JBOSS_JMX_CONSOLE, "JBoss security baseline - Auth constraint for /jmx-console/HTMLAdaptor is not enabled in " + path + "(" + tagName + " is missing or wrong)", true));
    }
}
