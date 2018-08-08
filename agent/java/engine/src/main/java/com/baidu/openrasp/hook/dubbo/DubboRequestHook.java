package com.baidu.openrasp.hook.dubbo;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.tool.Reflection;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

/**
 * Created by anyang on 2018/6/20.
 */
public class DubboRequestHook extends AbstractClassHook {

    public DubboRequestHook() {
        couldIgnore = false;
    }

    @Override
    public boolean isClassMatched(String className) {
        return "com/alibaba/dubbo/rpc/filter/ContextFilter".equals(className);
    }

    @Override
    public String getType() {
        return "request";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {

        String src = getInvokeStaticSrc(DubboRequestHook.class, "getDubboRpcRequestParameters",
                "$2", Object.class);
        insertBefore(ctClass, "invoke", null, src);
    }

    public static void getDubboRpcRequestParameters(Object object) {

        try {

            Object[] args = (Object[]) Reflection.invokeMethod(object, "getArguments", new Class[]{});
            Class<?>[] parameterTypes = (Class<?>[]) Reflection.invokeMethod(object, "getParameterTypes", new Class[]{});
            Map<String, String[]> map = new HashMap<String, String[]>(args.length);
            if (args.length != 0) {
                for (int i = 0; i < args.length; i++) {
                    if (parameterTypes[i].isPrimitive() || isWrapClass(parameterTypes[i]) || args[i] instanceof String) {
                        String[] strings = new String[1];
                        strings[0] = String.valueOf(args[i]);
                        map.put(String.valueOf(i), strings);
                    }

                }
            }
            HookHandler.checkDubboFilterRequest(map);

        } catch (Exception e) {
            e.printStackTrace();
        }

    }

    public static boolean isWrapClass(Class clazz) {
        try {
            return ((Class) clazz.getField("TYPE").get(null)).isPrimitive();
        } catch (Exception e) {

            e.printStackTrace();
        }
        return false;
    }
}
