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

package com.baidu.openrasp.hook;


import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.util.HashMap;

/**
 * Created by tyy on 17-11-17.
 * 检测socket连接的hook点
 */
public class SocketHook extends AbstractClassHook {

    /**
     * (none-javadoc)
     *
     * @see AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return className.equals("java/net/Socket");
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "socket";
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(SocketHook.class, "checkSocketHost",
                "$1", SocketAddress.class);
        insertBefore(ctClass, "connect", "(Ljava/net/SocketAddress;I)V", src);
    }

    /**
     * 检测socket连接的host
     *
     * @param address socket连接地址
     */
    public static void checkSocketHost(SocketAddress address) {
        try {
            if (address != null && address instanceof InetSocketAddress) {
                HashMap<String, Object> params = new HashMap<String, Object>();
                String hostName = ((InetSocketAddress) address).getHostName();
                params.put("hostname", hostName);
                HookHandler.doCheck(CheckParameter.Type.SSRF, params);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
