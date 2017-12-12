/*
 * Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

package com.fuxi.javaagent.hook.ssrf;

import com.fuxi.javaagent.HookHandler;
import com.fuxi.javaagent.tool.Reflection;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;
import org.objectweb.asm.commons.AdviceAdapter;
import org.objectweb.asm.commons.Method;

/**
 * Created by tyy on 17-12-7.
 *
 * commons-httpclinet 框架的 http 请求 hook 点
 */
public class CommonHttpClientHook extends AbstractSSRFHook {
    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/commons/httpclient/HttpClient".equals(className);
    }

    @Override
    protected MethodVisitor hookMethod(int access, String name, String desc, String signature, String[] exceptions, MethodVisitor mv) {
        if (name.equals("executeMethod") && desc.equals("(Lorg/apache/commons/httpclient/HostConfiguration;" +
                "Lorg/apache/commons/httpclient/HttpMethod;" +
                "Lorg/apache/commons/httpclient/HttpState;)I")) {
            return new AdviceAdapter(Opcodes.ASM5, mv, access, name, desc) {
                @Override
                protected void onMethodEnter() {
                    loadArg(1);
                    invokeStatic(Type.getType(CommonHttpClientHook.class),
                            new Method("checkHttpConnection", "(Ljava/lang/Object;)V"));
                }
            };
        }
        return mv;
    }

    public static void checkHttpConnection(Object httpMethod) {
        try {
            if (httpMethod != null) {
                Object uri = Reflection.invokeMethod(httpMethod, "getURI", new Class[]{});
                if (uri != null) {
                    String host = Reflection.invokeStringMethod(uri, "getHost", new Class[]{});
                    checkHttpUrl(uri.toString(), host,"commons_http_client");
                }
            }
        } catch (Throwable t) {
            HookHandler.LOGGER.warn(t.getMessage());
        }
    }
}
