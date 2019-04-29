package com.baidu.openrasp.plugin.v8;

import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;
import com.jsoniter.spi.JsoniterSpi;
import com.jsoniter.extra.Base64Support;
import com.jsoniter.output.JsonStream;
import com.jsoniter.JsonIterator;
import com.jsoniter.any.Any;
import org.apache.log4j.Logger;
import java.io.File;
import java.io.FileFilter;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.config.Config;
import org.apache.commons.io.filefilter.FileFilterUtils;
import org.apache.commons.io.FileUtils;
import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.plugin.info.AttackInfo;
import com.baidu.openrasp.plugin.info.EventInfo;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.checker.CheckParameter.Type;
import org.scijava.nativelib.NativeLoader;
import com.baidu.openrasp.tool.filemonitor.FileScanListener;
import com.baidu.openrasp.tool.filemonitor.FileScanMonitor;

public class V8 {
    public static final Logger PLUGIN_LOGGER = Logger.getLogger(V8.class.getPackage().getName() + ".log");
    public static final Logger LOGGER = Logger.getLogger(V8.class.getPackage().getName());
    public static Integer watchId = null;

    static {
        JsoniterSpi.setDefaultConfig(new com.jsoniter.spi.Config.Builder().escapeUnicode(false).build());
        Base64Support.enable();
    }

    public synchronized static void Init() throws Exception {
        NativeLoader.loadLibrary("openrasp_v8_java");
        Initialize();
        if (!CloudUtils.checkCloudControlEnter()) {
            UpdatePlugin();
            InitFileWatcher();
        }
    }

    public synchronized static void Release() {
        if (watchId != null) {
            boolean oldValue = HookHandler.enableHook.getAndSet(false);
            FileScanMonitor.removeMonitor(watchId);
            watchId = null;
            HookHandler.enableHook.set(oldValue);
        }
    }

    public synchronized static native boolean Initialize();

    public synchronized static native boolean Dispose();

    public synchronized static native boolean CreateSnapshot(String config, Object[] plugins);

    public static native String Check(String type, byte[] params, int params_size, Context context,
            boolean new_request);

    public static native String ExecuteScript(String source, String filename) throws Exception;

    public static void PluginLog(String log) {
        PLUGIN_LOGGER.info(log.replaceAll("\n$", ""));
    }

    public static List<EventInfo> Check(CheckParameter checkParameter) {
        Type type = checkParameter.getType();
        V8ByteArrayOutputStream params = new V8ByteArrayOutputStream();
        JsonStream.serialize(checkParameter.getParams(), params);

        int hashcode = 0;
        if (type == Type.DIRECTORY || type == Type.READFILE || type == Type.WRITEFILE || type == Type.SQL) {
            hashcode = ByteBuffer.wrap(params.getByteArray()).hashCode();
        }
        if (hashcode != 0) {
            if (Config.commonLRUCache.isContainsKey(hashcode)) {
                return null;
            }
        }

        String results = null;
        try {
            results = V8.Check(type.getName(), params.getByteArray(), params.size(),
            new Context(checkParameter.getRequest()), type == Type.REQUEST);
        } catch (Exception e) {
            String message = e.getMessage();
            int errorCode = ErrorType.PLUGIN_ERROR.getCode();
            LOGGER.error(CloudUtils.getExceptionObject(message, errorCode), e);
            return null;
        }

        if (results == null) {
            if (hashcode != 0 && Config.commonLRUCache.maxSize() != 0) {
                Config.commonLRUCache.put(hashcode, null);
            }
            return null;
        }

        Any any = JsonIterator.deserialize(results);
        if (any == null) {
            return null;
        }
        ArrayList<EventInfo> attackInfos = new ArrayList<EventInfo>();
        for (Any rst : any.asList()) {
            attackInfos.add(new AttackInfo(checkParameter, rst.toString("action"), rst.toString("message"),
                    rst.toString("name"), rst.toString("algorithm"), rst.toInt("confidence")));
        }
        return attackInfos;
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
                    scripts.add(new String[] { name, source });
                } catch (Exception e) {
                    String message = e.getMessage();
                    int errorCode = ErrorType.PLUGIN_ERROR.getCode();
                    LOGGER.error(CloudUtils.getExceptionObject(message, errorCode), e);
                }
            }
        }
        HookHandler.enableHook.set(oldValue);
        return UpdatePlugin(scripts);
    }

    public synchronized static boolean UpdatePlugin(String name, String content) {
        List<String[]> scripts = new ArrayList<String[]>();
        scripts.add(new String[] { name, content });
        return UpdatePlugin(scripts);
    }

    public synchronized static boolean UpdatePlugin(List<String[]> scripts) {
        boolean rst = CreateSnapshot("{}", scripts.toArray());
        if (rst) {
            try {
                String jsonString = ExecuteScript("JSON.stringify(RASP.algorithmConfig || {})", "get-algorithm-config.js");
                jsonString = new JsonParser().parse(jsonString).getAsString();
                Config.getConfig().setConfig("algorithm.config", jsonString, true);
            } catch (Exception e) {
                String message = e.getMessage();
                int errorCode = ErrorType.PLUGIN_ERROR.getCode();
                LOGGER.error(CloudUtils.getExceptionObject(message, errorCode), e);
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
