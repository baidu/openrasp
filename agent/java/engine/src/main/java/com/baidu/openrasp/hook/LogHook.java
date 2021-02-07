/*
 * Copyright 2017-2021 Baidu Inc.
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

package com.baidu.openrasp.hook;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;
import org.apache.commons.lang3.StringUtils;

import java.io.IOException;
import java.util.HashMap;

/**
 * @description: 敏感日志检测hook点
 * @author: anyang
 * @create: 2019/06/12 14:11
 */
public class LogHook extends AbstractClassHook {
    private static final String LOG4J = "log4j";
    private static final String LOG4J2 = "log4j2";
    private static final String LOGBACK = "logback";
    private String type;

    @Override
    public boolean isClassMatched(String className) {
        if (checkClassName(className)) {
            this.type = LOG4J;
            return true;
        } else if ("org/apache/logging/log4j/spi/AbstractLogger".equals(className)) {
            this.type = LOG4J2;
            return true;
        } else if ("ch/qos/logback/classic/Logger".equals(className)) {
            this.type = LOGBACK;
            return true;
        }
        return false;
    }

    @Override
    public String getType() {
        return "log";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src1 = getInvokeStaticSrc(LogHook.class, "checkLogMessage",
                "\"" + type + "\"" + ",$3", String.class, Object.class);
        String src2 = getInvokeStaticSrc(LogHook.class, "checkLogMessage",
                "\"" + type + "\"" + ",$4", String.class, Object.class);
        if (this.type.equals(LOG4J)) {
            insertBefore(ctClass, "forcedLog", null, src1);
        } else if (this.type.equals(LOG4J2)) {
            insertBefore(ctClass, "logMessageSafely", null, src2);
        } else if (this.type.equals(LOGBACK)) {
            insertBefore(ctClass, "buildLoggingEventAndAppend", null, src2);
        }
    }

    public static void checkLogMessage(String type, Object message) {
        if (message != null) {
            Object logMessage = message;
            if (LOG4J2.equals(type)) {
                logMessage = Reflection.invokeStringMethod(message, "getFormattedMessage", new Class[]{});
            }
            if (logMessage != null) {
                HashMap<String, Object> params = new HashMap<String, Object>();
                if (!(logMessage instanceof String)) {
                    logMessage = logMessage.toString();
                }
                params.put("message", logMessage);
                HookHandler.doCheck(CheckParameter.Type.POLICY_LOG, params);
            }
        }
    }

    private boolean checkClassName(String className) {
        if (StringUtils.startsWith(className, "org/apache")
                && StringUtils.endsWith(className, "log4j/Category")) {
            String[] strs = className.split("/");
            return strs.length == 4 && strs[0].equals("org") && strs[1].equals("apache")
                    && strs[2].equals("log4j") && strs[3].equals("Category");
        }
        return false;
    }
}
