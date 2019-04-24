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

package com.baidu.openrasp.detector;

import com.baidu.openrasp.tool.model.ApplicationModel;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.security.ProtectionDomain;

/**
 * Created by tyy on 19-2-12.
 */
public class WebsphereDetector extends ServerDetector {


    private static final String WAS_INSTALL_ROOT = "was.install.root";
    private static final String WAS_VERSION_ROOT = File.separator + "properties" + File.separator + "version" + File.separator + "WAS.product";

    @Override
    public boolean isClassMatched(String className) {
        return "org/eclipse/core/launcher/Main".equals(className);
    }

    @Override
    public boolean handleServerInfo(ClassLoader classLoader, ProtectionDomain domain) {
        String version = "";
        try {
            String wasInstallRoot = System.getProperty(WAS_INSTALL_ROOT);
            if (wasInstallRoot != null) {
                Document root = getXMLDocument(wasInstallRoot + WAS_VERSION_ROOT);
                if (root != null) {
                    NodeList product = root.getElementsByTagName("product");
                    if (product != null && product.getLength() > 0) {
                        Element element = (Element) product.item(0);
                        NodeList versions = element.getElementsByTagName("version");
                        if (versions != null && versions.getLength() > 0) {
                            version = versions.item(0).getTextContent();
                        }
                    }
                }
            }
        } catch (Throwable t) {
            logDetectError("handle websphere startup failed", t);
        }
        ApplicationModel.setServerInfo("websphere", version);
        return true;

    }

    private Document getXMLDocument(String path) {
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            factory.setValidating(false);
            DocumentBuilder builder = factory.newDocumentBuilder();
            builder.setEntityResolver(new IgnoreDTDEntityResolver());
            return builder.parse(new File(path));
        } catch (Exception e) {
            logDetectError("parse websphere xml failed", e);
        }
        return null;
    }

    class IgnoreDTDEntityResolver implements EntityResolver {
        @Override
        public InputSource resolveEntity(String publicId, String systemId) {
            return new InputSource(new ByteArrayInputStream("<?xml version='1.0' encoding='UTF-8'?>".getBytes()));
        }
    }

}
