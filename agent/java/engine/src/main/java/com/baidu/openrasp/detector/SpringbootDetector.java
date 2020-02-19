package com.baidu.openrasp.detector;

import com.baidu.openrasp.tool.model.ApplicationModel;

import java.security.ProtectionDomain;

/**
 * @description: 标识tomcat启动方式
 * @author: anyang
 * @create: 2019/04/22 11:35
 */
public class SpringbootDetector extends ServerDetector{
    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/catalina/startup/Bootstrap".equals(className);
    }

    @Override
    public boolean handleServerInfo(ClassLoader classLoader, ProtectionDomain domain) {
        //tomcat标准启动方式设置为true，内置tomcat启动设置为false
        ApplicationModel.setStartUpInfo("true");
        return false;
    }
}
