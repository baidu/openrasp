package com.baidu.openrasp.hook;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.checker.policy.JBossJMXSecurityChecker;
import com.baidu.openrasp.tool.Reflection;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * 　　* @Description: JBoss的JMX Console 安全检查
 * 　　* @author anyang
 * 　　* @date 2018/7/30 15:46
 */
public class JBossJMXConsoleHook extends AbstractClassHook {

    public static String serverVersion = null;
    public String className;

    @Override
    public boolean isClassMatched(String className) {
        if ("org/jboss/bootstrap/microcontainer/ServerImpl".equals(className)) {
            this.className = className;
        }
        return "org/jboss/system/server/ServerImpl".equals(className) ||
                "org/jboss/bootstrap/microcontainer/ServerImpl".equals(className) ||
                "org/jboss/bootstrap/impl/as/server/AbstractJBossASServerBase".equals(className);
    }

    @Override
    public String getType() {
        return "JBossJMXConsole";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {

        String src = getInvokeStaticSrc(JBossJMXConsoleHook.class, "checkJBossJMXConsole", "$0", Object.class);
        if ("org/jboss/bootstrap/microcontainer/ServerImpl".equals(className)) {

            insertBefore(ctClass, "doStart", null, src);
        } else {
            insertBefore(ctClass, "start", null, src);
        }


    }

    /**
     * JBoss启动时检测JMX Console配置
     */
    public static void checkJBossJMXConsole(Object object) {

        try {
            serverVersion = Reflection.invokeStringMethod(object, "getVersionNumber", new Class[]{});
        } catch (Exception e) {

            JBossJMXSecurityChecker.LOGGER.error(JBossJMXSecurityChecker.JBOSS_SECURITY_CHECK_ERROR + " :" + "JBoss 支持版本4.x-6.x", e);
        }
        HookHandler.doCheckWithoutRequest(CheckParameter.Type.POLICY_JBOSS_JMX_CONSOLE, CheckParameter.EMPTY_MAP);
    }
}
