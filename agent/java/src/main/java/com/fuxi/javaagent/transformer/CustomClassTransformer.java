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

package com.fuxi.javaagent.transformer;

import com.fuxi.javaagent.config.Config;
import com.fuxi.javaagent.hook.*;
import org.apache.log4j.Logger;

import java.lang.instrument.ClassFileTransformer;
import java.lang.instrument.IllegalClassFormatException;
import java.security.ProtectionDomain;
import java.util.HashMap;
import java.util.HashSet;

/**
 * 自定义类字节码转换器，用于hook类德 方法
 */
public class CustomClassTransformer implements ClassFileTransformer {
    private static final Logger LOGGER = Logger.getLogger(CustomClassTransformer.class.getName());
    private static HashMap<String, ClassLoader> classLoaderCache = new HashMap<String, ClassLoader>();
    private HashSet<AbstractClassHook> hooks;

    public CustomClassTransformer() {
        hooks = new HashSet<AbstractClassHook>();

        addHook(new WebDAVCopyResourceHook());
        addHook(new CoyoteInputStreamHook());
        addHook(new DeserializationHook());
        addHook(new DiskFileItemHook());
        addHook(new FileHook());
        addHook(new FileInputStreamHook());
        addHook(new FileOutputStreamHook());
        addHook(new OgnlHook());
        addHook(new ProcessBuilderHook());
        addHook(new Struts2DispatcherHook());
        addHook(new SQLDriverManagerHook());
        addHook(new SQLStatementHook());
        addHook(new SQLResultSetHook());
        addHook(new WeblogicJspBaseHook());
        addHook(new XXEHook());
        addHook(new JspCompilationContextHook());
        addHook(new TomcatStartupHook());
        addHook(new ApplicationFilterHook());
        addHook(new ReflectionHook());
        addHook(new JettyServerHandleHook());
        addHook(new JettyHttpInputHook());
        addHook(new JettyServerHook());
        addHook(new CoyoteAdapterHook());
        addHook(new ProxyDirContextHook());
        addHook(new JspIncludeHook());
        addHook(new JstlImportHook());
    }

    private void addHook(AbstractClassHook hook) {
        String[] ignore = Config.getConfig().getIgnoreHooks();
        for (String s : ignore) {
            if (s.equals("all") || s.equals(hook.getType())) {
                LOGGER.info("ignore hook type " + hook.getType());
                return;
            }
        }
        hooks.add(hook);
    }

    /**
     * 过滤需要hook的类，进行字节码更改
     *
     * @see ClassFileTransformer#transform(ClassLoader, String, Class, ProtectionDomain, byte[])
     */
    @Override
    public byte[] transform(ClassLoader loader, String className, Class<?> classBeingRedefined,
                            ProtectionDomain domain, byte[] classfileBuffer) throws IllegalClassFormatException {
        for (final AbstractClassHook hook : hooks) {
            if (hook.isClassMatched(className)) {
                return hook.transformClass(className, classfileBuffer);
            }
        }
        handleClassLoader(loader, className);
        return null;
    }

    public static ClassLoader getClassLoader(String className) {
        return classLoaderCache.get(className);
    }

    private static void handleClassLoader(ClassLoader loader, String className) {
        if (className.equals("org/apache/catalina/util/ServerInfo")) {
            classLoaderCache.put(className.replace('/', '.'), loader);
        }
    }
}
