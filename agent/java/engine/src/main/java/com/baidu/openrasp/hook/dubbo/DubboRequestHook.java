/*
 * Copyright 2017-2019 Baidu Inc.
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

package com.baidu.openrasp.hook.dubbo;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

/**
 * @author anyang
 * @Description: dubbo的请求hook点
 * @date 2018/8/1315:13
 */
@HookAnnotation
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
        return "dubboRequest";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {

        String src = getInvokeStaticSrc(DubboRequestHook.class, "getDubboRpcRequestParameters",
                "$2", Object.class);
        insertBefore(ctClass, "invoke", null, src);
    }

    public static void getDubboRpcRequestParameters(Object object) {
        Object[] args = (Object[]) Reflection.invokeMethod(object, "getArguments", new Class[]{});
        Class<?>[] parameterTypes = (Class<?>[]) Reflection.invokeMethod(object, "getParameterTypes", new Class[]{});
        Map<String, String[]> map = new HashMap<String, String[]>(args.length);
        if (args.length != 0) {
            for (int i = 0; i < args.length; i++) {
                if (parameterTypes[i].isPrimitive() || isWrapClass(parameterTypes[i]) || args[i] instanceof String) {
                    String[] strings = new String[1];
                    strings[0] = String.valueOf(args[i]);
                    map.put("dubbo-" + i, strings);
                }

            }
        }

        HookHandler.checkDubboFilterRequest(map);

    }

    public static boolean isWrapClass(Class clazz) {
        try {
            return ((Class) clazz.getField("TYPE").get(null)).isPrimitive();
        } catch (Exception e) {
            return false;
        }
    }
}
