package com.baidu.openrasp.plugin.checker.policy;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.server.jboss.JBossStartupHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.info.EventInfo;
import com.baidu.openrasp.plugin.info.SecurityPolicyInfo;
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
    private static final String HTTP_METHOD = "http-method";
    private static final String AUTH_CONSTRAINT = "auth-constraint";
    private static final String[] WEB_RESOURCE_NAME_VALUES = new String[]{"htmladaptor"};
    private static final String[] URL_PATTERN_VALUES = new String[]{"/*"};
    private static final String[] HTTP_METHOD_VALUES = new String[]{"get", "post"};
    public static final Logger LOGGER = Logger.getLogger(HookHandler.class.getName());
    public static HashMap<String, String> map = new HashMap<String, String>();

    static {
        map.put(SECURITY_DOMAIN, "SECURITY_DOMAIN");
        map.put(SECURITY_CONSTRAINT, "SECURITY_CONSTRAINT");
        map.put(WEB_RESOURCE_COLLECTION, "WEB_RESOURCE_COLLECTION");
        map.put(WEB_RESOURCE_NAME, "WEB_RESOURCE_NAME");
        map.put(URL_PATTERN, "URL_PATTERN");
        map.put(HTTP_METHOD, "HTTP_METHOD");
        map.put(AUTH_CONSTRAINT, "AUTH_CONSTRAINT");
    }

    @Override
    public List<EventInfo> checkParam(CheckParameter checkParameter) {

        String jbossBaseDir = System.getProperty("jboss.home.dir");
        List<EventInfo> infos = new LinkedList<EventInfo>();
        String serverVersion = JBossStartupHook.serverVersion;
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

                LOGGER.error(JBOSS_SECURITY_CHECK_ERROR + " :" + "JBoss 支持版本4.x-7.x");
            }
            checkJBossWebXml(jbossWebXmlPath, infos);
            checkWebXml(webXmlPath, infos);
        } else {
            LOGGER.error(JBOSS_SECURITY_CHECK_ERROR + " :" + "JBoss 支持版本4.x-7.x");
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
                handleError(SECURITY_DOMAIN, infos);
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
                        checkXmlElement(subElement, WEB_RESOURCE_NAME, Arrays.asList(WEB_RESOURCE_NAME_VALUES), infos);
                        checkXmlElement(subElement, URL_PATTERN, Arrays.asList(URL_PATTERN_VALUES), infos);
                        checkXmlElement(subElement, HTTP_METHOD, Arrays.asList(HTTP_METHOD_VALUES), infos);
                    } else {
                        handleError(WEB_RESOURCE_COLLECTION, infos);
                    }

                    NodeList authConstraint = element.getElementsByTagName("auth-constraint");
                    if (authConstraint.getLength() == 0) {
                        handleError(AUTH_CONSTRAINT, infos);
                    }
                }

            } else {
                handleError(SECURITY_CONSTRAINT, infos);
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

    public void checkXmlElement(Element element, String key, List<String> reference, List<EventInfo> infos) {
        NodeList list = element.getElementsByTagName(key);
        if (list.getLength() > 0) {
            for (int i = 0; i < list.getLength(); i++) {
                if (!reference.contains(list.item(i).getTextContent().toLowerCase())) {
                    handleError(key, infos);
                }
            }

        } else {
            handleError(key, infos);
        }
    }


    public void handleError(String tagName, List<EventInfo> infos) {

        infos.add(new SecurityPolicyInfo(SecurityPolicyInfo.Type.valueOf(map.get(tagName)), "jboss未配置" + tagName + "或者配置错误", true));
    }
}
