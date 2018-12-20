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
import com.strobel.assembler.metadata.MetadataSystem;
import com.strobel.assembler.metadata.TypeReference;
import com.strobel.decompiler.DecompilationOptions;
import com.strobel.decompiler.DecompilerSettings;
import com.strobel.decompiler.languages.BytecodeOutputOptions;
import com.strobel.decompiler.languages.java.JavaFormattingOptions;

import java.io.*;
import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * @description: 反编译工具类
 * @author: anyang
 * @create: 2018/10/18 20:50
 */
public class Decompiler {
    private static LRUCache<String, String> decompileCache = new LRUCache<String, String>(100);
    private static LRUCache<String, Long> fileLastModify = new LRUCache<String, Long>(100);

    private static String getDecompilerString(File orinalFile) {
        if (orinalFile.exists()) {
            DecompilerSettings settings = new DecompilerSettings();
            settings.setBytecodeOutputOptions(BytecodeOutputOptions.createVerbose());
            if (settings.getJavaFormattingOptions() == null) {
                settings.setJavaFormattingOptions(JavaFormattingOptions.createDefault());
            }
            settings.setShowDebugLineNumbers(true);
            DecompilationOptions decompilationOptions = new DecompilationOptions();
            decompilationOptions.setSettings(settings);
            decompilationOptions.setFullDecompilation(true);
            RaspTypeLoader typeLoader = new RaspTypeLoader();
            MetadataSystem metadataSystem = new MetadataSystem(typeLoader);
            String path = orinalFile.getPath().replaceAll("\\\\", "/");
            TypeReference type = metadataSystem.lookupType(path);
            DecompilerProvider newProvider = new DecompilerProvider();
            newProvider.setDecompilerReferences(settings, decompilationOptions);
            newProvider.setType(type.resolve());
            newProvider.generateContent();
            return newProvider.getTextContent();
        }
        return "";
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

    public static Map<String, String> getAlarmPoint(StackTraceElement[] stackTraceElements, String appBasePath) {
        Map<String, String> result = new HashMap<String, String>();
        StackTraceFilter traceFilter = new StackTraceFilter();
        traceFilter.filter(stackTraceElements);
        for (Map.Entry<String, Integer> entry : traceFilter.class_lineNumber.entrySet()) {
            String description = entry.getKey() + "." + traceFilter.class_method.get(entry.getKey()) + "(" + entry.getValue() + ")";
            List<String> paths = searchFiles(new File(appBasePath), entry.getKey());
            for (String path : paths) {
                File originFile = new File(path);
                if (decompileCache.isContainsKey(description)) {
                    if (fileLastModify.isContainsKey(entry.getKey()) && fileLastModify.get(entry.getKey()) == originFile.lastModified()) {
                        result.put(description, decompileCache.get(description));
                        continue;
                    }
                }
                String src = Decompiler.getDecompilerString(originFile);
                if (!src.isEmpty()) {
                    for (String line : src.split(System.getProperty("line.separator"))) {
                        String matched = Decompiler.matchStringByRegularExpression(line, traceFilter.class_lineNumber.get(entry.getKey()));
                        if (!"".equals(matched)) {
                            result.put(description, matched);
                            decompileCache.put(description, matched);
                            fileLastModify.put(entry.getKey(), originFile.lastModified());
                            break;
                        }
                    }
                }
            }
        }
        return result;
    }

    private static List<String> searchFiles(File folder, final String fileName) {
        final String simpleName = fileName.substring(fileName.lastIndexOf(".") + 1) + ".class";
        List<String> result = new ArrayList<String>();
        if (folder.isFile()) {
            if (folder.getAbsolutePath().contains(fileName.replace(".", File.separator))) {
                result.add(folder.getAbsolutePath());
            }
        }

        File[] subFolders = folder.listFiles(new FileFilter() {
            public boolean accept(File file) {
                return file.isDirectory() || file.getName().equals(simpleName);
            }
        });
        if (subFolders != null) {
            for (File file : subFolders) {
                if (file.isFile()) {
                    if (file.getAbsolutePath().contains(fileName.replace(".", File.separator))) {
                        result.add(file.getAbsolutePath());
                    }
                } else {
                    result.addAll(searchFiles(file, fileName));
                }
            }
        }
        return result;
    }
}
