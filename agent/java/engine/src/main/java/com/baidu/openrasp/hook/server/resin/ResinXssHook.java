/*
 * Copyright 2017-2018 Baidu Inc.
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

package com.baidu.openrasp.hook.server.resin;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.hook.server.ServerXssHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.Annotation.HookAnnotation;
import com.baidu.openrasp.tool.hook.ServerXss;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;
import java.util.HashMap;


/**
　　* @Description: resin xss检测的hook点
　　* @author anyang
　　* @date 2018/8/7 19:27
　　*/
@HookAnnotation
public class ResinXssHook extends ServerXssHook {

    @Override
    public boolean isClassMatched(String className) {
        return "com/caucho/server/http/ToByteResponseStream".equals(className) ||
                "com/caucho/server/connection/ToByteResponseStream".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {

        String src = getInvokeStaticSrc(ResinXssHook.class, "getResinOutputBuffer", "_charBuffer,_charLength,_isOutputStreamOnly", char[].class, int.class, boolean.class);
        insertBefore(ctClass, "flushCharBuffer", "()V", src);

    }

    public static void getResinOutputBuffer(char[] buffer, int len, boolean isOutputStreamOnly) {

        if (len > 0 && !isOutputStreamOnly) {
            char[] temp = new char[len];
            System.arraycopy(buffer, 0, temp, 0, len);
            String content = new String(temp);
            try {
                HashMap<String, Object> params = ServerXss.generateXssParameters(content);
                HookHandler.doCheck(CheckParameter.Type.XSS, params);

            } catch (Exception e) {

                e.printStackTrace();
            }
        }

    }
}
