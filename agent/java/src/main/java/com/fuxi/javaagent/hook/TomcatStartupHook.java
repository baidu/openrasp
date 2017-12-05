package com.fuxi.javaagent.hook;

import com.fuxi.javaagent.HookHandler;
import com.fuxi.javaagent.plugin.checker.CheckParameter;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;
import org.objectweb.asm.commons.AdviceAdapter;
import org.objectweb.asm.commons.Method;

/**
 * Created by tyy on 8/9/17.
 * 用于hook tomcat启动函数
 */
public class TomcatStartupHook extends AbstractClassHook {

    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/catalina/startup/Catalina".equals(className);
    }

    @Override
    public String getType() {
        return "startup";
    }

    @Override
    protected MethodVisitor hookMethod(int access, String name, String desc, String signature, String[] exceptions, MethodVisitor mv) {
        if (name.equals("start")) {
            return new AdviceAdapter(Opcodes.ASM5, mv, access, name, desc) {
                @Override
                protected void onMethodEnter() {
                    invokeStatic(Type.getType(TomcatStartupHook.class),
                            new Method("checkTomcatStartup", "()V"));
                }
            };
        }
        return mv;
    }

    /**
     * tomcat启动时检测安全规范
     */
    public static void checkTomcatStartup() {
        HookHandler.doCheckWithoutRequest(CheckParameter.Type.POLICY_TOMCAT_START, CheckParameter.EMPTY_MAP);
    }
}
