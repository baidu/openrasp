package com.baidu.openrasp.hook.server.jboss;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.checker.policy.JBossJMXSecurityChecker;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.model.ApplicationModel;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * 　　* @Description: JBoss的JMX Console 安全检查
 * 　　* @author anyang
 * 　　* @date 2018/7/30 15:46
 */
public class JBossStartupHook extends AbstractClassHook {

    public static String serverVersion = null;

    @Override
    public boolean isClassMatched(String className) {
        return "org/jboss/system/server/ServerImpl".equals(className) ||
                "org/jboss/bootstrap/AbstractServerImpl".equals(className) ||
                "org/jboss/bootstrap/impl/base/server/AbstractServer".equals(className);
    }

    @Override
    public String getType() {
        return "JBossJMXConsole";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {

        String src = getInvokeStaticSrc(JBossStartupHook.class, "checkJBossJMXConsole", "$0", Object.class);
        insertBefore(ctClass, "start", null, src);
    }

    /**
     * JBoss启动时检测JMX Console配置
     */
    public static void checkJBossJMXConsole(Object object) {

        try {
            serverVersion = Reflection.invokeStringMethod(object, "getVersionNumber", new Class[]{});
            ApplicationModel.init("jboss", serverVersion);
            HookHandler.doCheckWithoutRequest(CheckParameter.Type.POLICY_JBOSS_JMX_CONSOLE, CheckParameter.EMPTY_MAP);
        } catch (Exception e) {
            JBossJMXSecurityChecker.LOGGER.error("handle jboss startup failed", e);
        }
    }
}
