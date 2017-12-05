/**
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

package com.fuxi.javaagent.hook;

import com.fuxi.javaagent.HookHandler;
import com.fuxi.javaagent.plugin.checker.CheckParameter;
import com.fuxi.javaagent.plugin.js.engine.JSContext;
import com.fuxi.javaagent.plugin.js.engine.JSContextFactory;
import com.fuxi.javaagent.tool.Reflection;
import org.mozilla.javascript.Scriptable;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;
import org.objectweb.asm.commons.AdviceAdapter;
import org.objectweb.asm.commons.Method;

/**
 * Created by lxk on 10/12/17.
 * All rights reserved
 */
public class WebDAVCopyResourceHook extends AbstractClassHook {
    /**
     * (none-javadoc)
     *
     * @see AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "webdav";
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/catalina/servlets/WebdavServlet".equals(className);
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractClassHook#hookMethod(int, String, String, String, String[], MethodVisitor)
     */
    @Override
    public MethodVisitor hookMethod(int access, String name, String desc,
                                    String signature, String[] exceptions, MethodVisitor mv) {
        if (name.equals("copyResource") && desc.startsWith("(Ljavax/naming/directory/DirContext;Ljava/util/Hashtable;Ljava/lang/String;Ljava/lang/String;)Z")) {
            return new AdviceAdapter(Opcodes.ASM5, mv, access, name, desc) {
                @Override
                protected void onMethodEnter() {
                    loadThis();
                    loadArg(2);
                    loadArg(3);
                    invokeStatic(Type.getType(WebDAVCopyResourceHook.class),
                            new Method("checkWebdavCopyResource", "(Ljava/lang/Object;Ljava/lang/String;Ljava/lang/String;)V"));
                }
            };
        }
        return mv;
    }

    /**
     * 检测WebDAV COPY MOVE
     *
     * @param webdavServlet
     * @param source
     * @param dest
     */
    public static void checkWebdavCopyResource(Object webdavServlet, String source, String dest) {
        if (webdavServlet != null && source != null && dest != null) {
            String realPath = null;
            try {
                Object servletContext = Reflection.invokeMethod(webdavServlet, "getServletContext", new Class[]{});
                realPath = Reflection.invokeStringMethod(servletContext, "getRealPath", new Class[]{String.class}, "/");
                realPath = realPath.endsWith(System.getProperty("file.separator")) ? realPath.substring(0, realPath.length() - 1) : realPath;
            } catch (Exception e) {
                e.printStackTrace();
            }
            if (realPath != null) {
                JSContext cx = JSContextFactory.enterAndInitContext();
                Scriptable params = cx.newObject(cx.getScope());
                params.put("source", params, realPath + source);
                params.put("dest", params, realPath + dest);
                HookHandler.doCheck(CheckParameter.Type.WEBDAV, params);
            }
        }
    }
}
