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

package com.baidu.openrasp.hook.server.websphere;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.hook.server.ServerStartupHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import com.baidu.openrasp.tool.model.ApplicationModel;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.IOException;

/**
 * @author anyang
 * @Description: 获取websphere的serverInfo
 * @date 2018/8/14 17:08
 */
@HookAnnotation
public class WebsphereStartupHook extends ServerStartupHook {

    private static final String WAS_INSTALL_ROOT = "was.install.root";
    private static final String WAS_VERSION_ROOT = File.separator + "properties" + File.separator + "version" + File.separator + "WAS.product";

    @Override
    public boolean isClassMatched(String className) {
        return "org/eclipse/core/launcher/Main".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(WebsphereStartupHook.class, "handleWebsphereStartup", "$0", Object.class);
        insertBefore(ctClass, "run", null, src);
    }

    public static void handleWebsphereStartup(Object object) {
        String wasIntsallRoot = System.getProperty(WAS_INSTALL_ROOT);
        if (wasIntsallRoot != null) {
            Document root = getXMLDocument(wasIntsallRoot + WAS_VERSION_ROOT);
            if (root != null) {
                NodeList product = root.getElementsByTagName("product");
                if (product != null && product.getLength() > 0) {
                    Element element = (Element) product.item(0);
                    NodeList versions = element.getElementsByTagName("version");
                    if (versions != null && versions.getLength() > 0) {
                        String version = versions.item(0).getTextContent();
                        if (version != null) {
                            ApplicationModel.init("websphere", version);
                        } else {
                            ApplicationModel.init("websphere", "");
                        }
                    }
                }
            }
        }
        sendRegister();
        if (!CloudUtils.checkCloudControlEnter()){
            HookHandler.doPolicyCheckWithoutRequest(CheckParameter.Type.POLICY_WEBSPHERE_START, CheckParameter.EMPTY_MAP);
        }
    }

    private static Document getXMLDocument(String path) {
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            factory.setValidating(false);
            DocumentBuilder builder = factory.newDocumentBuilder();
            builder.setEntityResolver(new IgnoreDTDEntityResolver());
            return builder.parse(new File(path));
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

    static class IgnoreDTDEntityResolver implements EntityResolver {
        @Override
        public InputSource resolveEntity(String publicId, String systemId) {
            return new InputSource(new ByteArrayInputStream("<?xml version='1.0' encoding='UTF-8'?>".getBytes()));
        }
    }
}