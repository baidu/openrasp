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

package com.baidu.openrasp.transformer;

import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.hook.*;
import com.baidu.openrasp.hook.catalina.*;
import com.baidu.openrasp.hook.file.DiskFileItemHook;
import com.baidu.openrasp.hook.file.FileHook;
import com.baidu.openrasp.hook.file.FileInputStreamHook;
import com.baidu.openrasp.hook.file.FileOutputStreamHook;
import com.baidu.openrasp.hook.jetty.*;
import com.baidu.openrasp.hook.sql.SQLDriverManagerHook;
import com.baidu.openrasp.hook.sql.SQLResultSetHook;
import com.baidu.openrasp.hook.sql.SQLStatementHook;
import com.baidu.openrasp.hook.ssrf.CommonHttpClientHook;
import com.baidu.openrasp.hook.ssrf.HttpClientHook;
import com.baidu.openrasp.hook.ssrf.URLConnectionHook;
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
        addHook(new CatalinaInputBufferHook());
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
        addHook(new JettyServerHandleHook());
        addHook(new JettyHttpInputHook());
        addHook(new JettyServerHook());
        addHook(new CoyoteAdapterHook());
        addHook(new ProxyDirContextHook());
        addHook(new JstlImportHook());
        addHook(new URLConnectionHook());
        addHook(new CommonHttpClientHook());
        addHook(new HttpClientHook());
        addHook(new CatalinaOutputBufferHook());
        addHook(new JettyHttpOutputHook());
        addHook(new CatalinaRequestHook());
        addHook(new JettyRequestHook());
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

    public HashSet<AbstractClassHook> getHooks() {
        return hooks;
    }

}
