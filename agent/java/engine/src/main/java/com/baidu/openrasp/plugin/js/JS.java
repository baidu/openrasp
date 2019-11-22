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

package com.baidu.openrasp.plugin.js;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.messaging.ErrorType;
import com.baidu.openrasp.messaging.LogTool;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.checker.CheckParameter.Type;
import com.baidu.openrasp.plugin.info.AttackInfo;
import com.baidu.openrasp.plugin.info.EventInfo;
import com.baidu.openrasp.tool.StackTrace;
import com.baidu.openrasp.tool.filemonitor.FileScanListener;
import com.baidu.openrasp.tool.filemonitor.FileScanMonitor;
import com.baidu.openrasp.tool.model.BuildRASPModel;
import com.baidu.openrasp.v8.ByteArrayOutputStream;
import com.baidu.openrasp.v8.V8;
import com.jsoniter.JsonIterator;
import com.jsoniter.any.Any;
import com.jsoniter.extra.Base64Support;
import com.jsoniter.output.JsonStream;
import org.apache.commons.io.FileUtils;
import org.apache.commons.io.filefilter.FileFilterUtils;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.FileFilter;
import java.nio.ByteBuffer;
import java.security.MessageDigest;
import java.util.ArrayList;
import java.util.List;

public class JS {
    public static final Logger PLUGIN_LOGGER = Logger.getLogger(JS.class.getPackage().getName() + ".log");
    public static final Logger LOGGER = Logger.getLogger(JS.class.getPackage().getName());
    public static Integer watchId = null;

    static {
        Base64Support.enable();
    }

    public synchronized static boolean Initialize() {
        try {
            V8.Load();
            if (!V8.Initialize()) {
                throw new Exception("[OpenRASP] Failed to initialize V8 worker threads");
            }
            V8.SetLogger(new com.baidu.openrasp.v8.Logger() {
                @Override
                public void log(String msg) {
                    PLUGIN_LOGGER.info(msg);
                }
            });
            V8.SetStackGetter(new com.baidu.openrasp.v8.StackGetter() {
                @Override
                public byte[] get() {
                    try {
                        ByteArrayOutputStream stack = new ByteArrayOutputStream();
                        JsonStream.serialize(StackTrace.getParamStackTraceArray(), stack);
                        stack.write(0);
                        return stack.getByteArray();
                    } catch (Exception e) {
                        return null;
                    }
                }
            });
            Context.setKeys();
            if (!CloudUtils.checkCloudControlEnter()) {
                UpdatePlugin();
                InitFileWatcher();
            }
            return true;
        } catch (Exception e) {
            e.printStackTrace();
            LOGGER.error(e);
            return false;
        }
    }

    public synchronized static void Dispose() {
        if (watchId != null) {
            boolean oldValue = HookHandler.enableHook.getAndSet(false);
            FileScanMonitor.removeMonitor(watchId);
            watchId = null;
            HookHandler.enableHook.set(oldValue);
        }
    }

    public static List<EventInfo> Check(CheckParameter checkParameter) {
        Type type = checkParameter.getType();
        ByteArrayOutputStream params = new ByteArrayOutputStream();
        JsonStream.serialize(checkParameter.getParams(), params);

        ByteBuffer byteBuffer = null;
        if ((type == Type.DIRECTORY || type == Type.READFILE || type == Type.WRITEFILE || type == Type.SQL || type == Type.SSRF)
                && params.getByteArray().length < 1000) {
            byteBuffer = ByteBuffer.wrap(params.getByteArray());
        }
        if (byteBuffer != null) {
            if (Config.commonLRUCache.isContainsKey(byteBuffer)) {
                return null;
            }
        }

        byte[] results = null;
        try {
            results = V8.Check(type.getName(), params.getByteArray(), params.size(),
                    new Context(checkParameter.getRequest()), type == Type.REQUEST, (int) Config.getConfig().getPluginTimeout());
        } catch (Exception e) {
            LogTool.error(ErrorType.PLUGIN_ERROR, e.getMessage(), e);
            return null;
        }

        if (results == null) {
            if (byteBuffer != null && Config.commonLRUCache.maxSize() != 0) {
                Config.commonLRUCache.put(byteBuffer, null);
            }
            return null;
        }

        try {
            Any any = JsonIterator.deserialize(results);
            if (any == null) {
                return null;
            }
            ArrayList<EventInfo> attackInfos = new ArrayList<EventInfo>();
            for (Any rst : any.asList()) {
                if (rst.toString("action").equals("exception")) {
                    PLUGIN_LOGGER.info(rst.toString("message"));
                } else {
                    attackInfos.add(new AttackInfo(checkParameter, rst.toString("action"), rst.toString("message"),
                            rst.toString("name"), rst.toString("algorithm"), rst.toInt("confidence")));
                }
            }
            return attackInfos;
        } catch (Exception e) {
            LOGGER.warn(e);
            return null;
        }
    }

    public synchronized static boolean UpdatePlugin() {
        boolean oldValue = HookHandler.enableHook.getAndSet(false);
        List<String[]> scripts = new ArrayList<String[]>();
        File pluginDir = new File(Config.getConfig().getScriptDirectory());
        LOGGER.debug("checker directory: " + pluginDir.getAbsolutePath());
        if (!pluginDir.isDirectory()) {
            pluginDir.mkdir();
        }
        FileFilter filter = FileFilterUtils.and(FileFilterUtils.sizeFileFilter(10 * 1024 * 1024, false),
                FileFilterUtils.suffixFileFilter(".js"));
        File[] pluginFiles = pluginDir.listFiles(filter);
        if (pluginFiles != null) {
            for (File file : pluginFiles) {
                try {
                    String name = file.getName();
                    String source = FileUtils.readFileToString(file, "UTF-8");
                    scripts.add(new String[]{name, source});
                } catch (Exception e) {
                    LogTool.error(ErrorType.PLUGIN_ERROR, e.getMessage(), e);
                }
            }
        }
        HookHandler.enableHook.set(oldValue);
        return UpdatePlugin(scripts);
    }

    public synchronized static boolean UpdatePlugin(String name, String content) {
        List<String[]> scripts = new ArrayList<String[]>();
        scripts.add(new String[]{name, content});
        return UpdatePlugin(scripts);
    }

    public synchronized static boolean UpdatePlugin(List<String[]> scripts) {
        boolean rst = V8.CreateSnapshot("{}", scripts.toArray(), BuildRASPModel.getRaspVersion());
        if (rst) {
            try {
                String jsonString = V8.ExecuteScript("JSON.stringify(RASP.algorithmConfig || {})", "get-algorithm-config.js");
                Config.getConfig().setConfig("algorithm.config", jsonString, true);
            } catch (Exception e) {
                LogTool.error(ErrorType.PLUGIN_ERROR, e.getMessage(), e);
            }
            Config.commonLRUCache.clear();
        }
        return rst;
    }

    public synchronized static void InitFileWatcher() throws Exception {
        boolean oldValue = HookHandler.enableHook.getAndSet(false);
        if (watchId != null) {
            FileScanMonitor.removeMonitor(watchId);
            watchId = null;
        }
        watchId = FileScanMonitor.addMonitor(Config.getConfig().getScriptDirectory(), new FileScanListener() {
            @Override
            public void onFileCreate(File file) {
                if (file.getName().endsWith(".js")) {
                    UpdatePlugin();
                }
            }

            @Override
            public void onFileChange(File file) {
                if (file.getName().endsWith(".js")) {
                    UpdatePlugin();
                }
            }

            @Override
            public void onFileDelete(File file) {
                if (file.getName().endsWith(".js")) {
                    UpdatePlugin();
                }
            }
        });
        HookHandler.enableHook.set(oldValue);
    }
}