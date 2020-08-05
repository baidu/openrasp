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

package com.baidu.openrasp.tool.decompile;

import com.baidu.openrasp.tool.LRUCache;
import com.baidu.openrasp.transformer.CustomClassTransformer;
import com.strobel.assembler.metadata.ArrayTypeLoader;
import com.strobel.assembler.metadata.MetadataSystem;
import com.strobel.assembler.metadata.TypeReference;
import com.strobel.decompiler.DecompilationOptions;
import com.strobel.decompiler.DecompilerSettings;
import com.strobel.decompiler.languages.BytecodeOutputOptions;
import com.strobel.decompiler.languages.java.JavaFormattingOptions;
import org.apache.commons.io.IOUtils;

import java.io.File;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * @description: 反编译工具类
 * @author: anyang
 * @create: 2018/10/18 20:50
 */
public class Decompiler {
    private static LRUCache<String, String> decompileCache = new LRUCache<String, String>(100);

    private static String getDecompilerString(InputStream in, String className) throws Exception {
        DecompilerSettings settings = new DecompilerSettings();
        settings.setBytecodeOutputOptions(BytecodeOutputOptions.createVerbose());
        if (settings.getJavaFormattingOptions() == null) {
            settings.setJavaFormattingOptions(JavaFormattingOptions.createDefault());
        }
        settings.setShowDebugLineNumbers(true);
        DecompilationOptions decompilationOptions = new DecompilationOptions();
        decompilationOptions.setSettings(settings);
        decompilationOptions.setFullDecompilation(true);
        ArrayTypeLoader typeLoader = new ArrayTypeLoader(IOUtils.toByteArray(in));
        MetadataSystem metadataSystem = new MetadataSystem(typeLoader);
        className = className.replace(".", "/");
        TypeReference type = metadataSystem.lookupType(className);
        DecompilerProvider newProvider = new DecompilerProvider();
        newProvider.setDecompilerReferences(settings, decompilationOptions);
        newProvider.setType(type.resolve());
        newProvider.generateContent();
        return newProvider.getTextContent();
    }

    private static String matchStringByRegularExpression(String line, int lineNumber) {
        String regex = ".*\\/\\*[E|S]L:" + lineNumber + "\\*\\/.*";
        Pattern p = Pattern.compile(regex);
        Matcher m = p.matcher(line);
        if (m.find()) {
            return m.group().trim().replaceAll("\\/\\/[^\\n]*|\\/\\*([^\\*^\\/]*|[\\*^\\/*]*|[^\\**\\/]*)*\\*+\\/", "");
        }
        return "";
    }

    public static ArrayList<String> getAlarmPoint(StackTraceElement[] stackTraceElements) {
        ArrayList<String> result = new ArrayList<String>();
        for (StackTraceElement element : stackTraceElements) {
            String className = element.getClassName();
            int lineNumber = element.getLineNumber();
            String description = element.toString();
            try {
                String simpleName = className.substring(className.lastIndexOf(".") + 1) + ".class";
                Class clazz = null;
                try {
                    clazz = Thread.currentThread().getContextClassLoader().loadClass(className);
                } catch (ClassNotFoundException e) {
                    ClassLoader classLoader = CustomClassTransformer.jspClassLoaderCache.get(className).get();
                    if (classLoader != null) {
                        try {
                            clazz = classLoader.loadClass(className);
                        } catch (Exception e1) {
                            CustomClassTransformer.jspClassLoaderCache.remove(className);
                        }
                    }
                }
                if (clazz != null) {
                    File file;
                    try {
                        file = new File(clazz.getResource(simpleName).getPath());
                    } catch (Exception e) {
                        CustomClassTransformer.jspClassLoaderCache.remove(className);
                        file = null;
                    }
                    if (file != null && decompileCache.isContainsKey(description + file.lastModified())) {
                        result.add(decompileCache.get(description + file.lastModified()));
                        continue;
                    }
                    String src = getDecompilerString(clazz.getResourceAsStream(simpleName), className);
                    if (!src.isEmpty()) {
                        boolean isFind = false;
                        for (String line : src.split(System.getProperty("line.separator"))) {
                            String matched = Decompiler.matchStringByRegularExpression(line, lineNumber);
                            if (!"".equals(matched)) {
                                isFind = true;
                                result.add(matched);
                                if (file != null) {
                                    decompileCache.put(description + file.lastModified(), matched);
                                }
                                break;
                            }
                        }
                        if (!isFind) {
                            result.add("");
                        }
                    } else {
                        result.add("");
                    }
                } else {
                    result.add("");
                }
            } catch (Throwable e) {
                result.add("");
            }

        }
        return result;
    }
}
