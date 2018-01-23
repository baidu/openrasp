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

package com.baidu.openrasp.hook.ssrf;

import com.baidu.openrasp.HookHandler;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;
import org.objectweb.asm.commons.AdviceAdapter;
import org.objectweb.asm.commons.Method;

import java.net.URI;

/**
 * Created by tyy on 17-12-8.
 *
 * httpclient 框架的请求 http 的 hook 点
 */
public class HttpClientHook extends AbstractSSRFHook {
    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/http/client/methods/HttpRequestBase".equals(className);
    }

    @Override
    protected MethodVisitor hookMethod(int access, String name, String desc, String signature, String[] exceptions, MethodVisitor mv) {
        if (name.equals("setURI") && desc.equals("(Ljava/net/URI;)V")) {
            return new AdviceAdapter(Opcodes.ASM5, mv, access, name, desc) {
                @Override
                protected void onMethodEnter() {
                    loadArg(0);
                    invokeStatic(Type.getType(HttpClientHook.class),
                            new Method("checkHttpUri", "(Ljava/net/URI;)V"));
                }
            };
        }
        return mv;
    }

    public static void checkHttpUri(URI uri) {
        String url = null;
        String hostName = null;
        try {
            if (uri != null) {
                url = uri.toString();
                hostName = uri.toURL().getHost();
            }
        } catch (Throwable t) {
            HookHandler.LOGGER.warn(t.getMessage());
        }
        if (hostName != null) {
            checkHttpUrl(url, hostName, "httpclient");
        }
    }
}
