package com.fuxi.javaagent.hook;

import com.fuxi.javaagent.HookHandler;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;
import org.objectweb.asm.commons.AdviceAdapter;
import org.objectweb.asm.commons.Method;

/**
 * Created by tyy on 9/25/17.
 */
public class JettyHttpInputHook extends AbstractClassHook {
    @Override
    public boolean isClassMatched(String className) {
        return className.equals("org/eclipse/jetty/server/HttpInput");
    }

    @Override
    public String getType() {
        return "body";
    }

    @Override
    protected MethodVisitor hookMethod(int access, String name, final String desc, String signature, String[] exceptions, MethodVisitor mv) {
        if (name.equals("read")) {
            return new AdviceAdapter(Opcodes.ASM5, mv, access, name, desc) {
                @Override
                protected void onMethodExit(int opcode) {
                    if (opcode == Opcodes.IRETURN) {
                        if (desc.equals("()I")) {
                            dup();
                            loadThis();
                            invokeStatic(Type.getType(HookHandler.class),
                                    new Method("onInputStreamRead", "(ILjava/lang/Object;)V"));
                        } else if (desc.equals("([BII)I")) {
                            dup();
                            loadThis();
                            loadArg(0);
                            loadArg(1);
                            loadArg(2);
                            invokeStatic(Type.getType(HookHandler.class),
                                    new Method("onInputStreamRead", "(ILjava/lang/Object;[BII)V"));
                        }
                    }
                    super.onMethodExit(opcode);
                }
            };
        }
        return mv;
    }
}
