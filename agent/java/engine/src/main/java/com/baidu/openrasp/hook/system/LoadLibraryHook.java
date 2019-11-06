package com.baidu.openrasp.hook.system;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.messaging.LogTool;
import com.baidu.openrasp.plugin.checker.CheckParameter;

import java.io.File;
import java.util.HashMap;

/**
 * Created by tyy on 19-7-23.
 *
 * hook 系统加载
 */
public abstract class LoadLibraryHook extends AbstractClassHook {

    static final ThreadLocal<Boolean> isLoadingLibrary = new ThreadLocal<Boolean>() {
        @Override
        protected Boolean initialValue() {
            return false;
        }
    };

    @Override
    public String getType() {
        return "loadLibrary";
    }

    public static void closeCheckSystemLoad() {
        isLoadingLibrary.set(false);
    }

    public static void checkSystemLoadFile(File libFile) {
        isLoadingLibrary.set(true);
        HashMap<String, Object> params = null;
        try {
            params = new HashMap<String, Object>();
            String realPath;
            // 判断 UNC 路径，如果是 UNC 不调用 getCanonicalPath，如果连不上 UNC 路径会有卡住的风险
            if (libFile.getPath().startsWith("//")
                    || libFile.getPath().startsWith("\\\\")) {
                realPath = libFile.getAbsolutePath();
            } else {
                try {
                    realPath = libFile.getCanonicalPath();
                } catch (Exception t) {
                    realPath = libFile.getAbsolutePath();
                }
            }
            params.put("function", "ClassLoader$NativeLibrary.init");
            params.put("path", libFile.getPath());
            params.put("realpath", realPath);
        } catch (Throwable t) {
            LogTool.traceHookWarn("system load hook check failed: " + t.getMessage(), t);
        }
        if (params != null) {
            HookHandler.doCheck(CheckParameter.Type.LOADLIBRARY, params);
        }
    }

    public static void checkSystemLoadPath(String libPath) {
        if (!isLoadingLibrary.get()) {
            HashMap<String, Object> params = null;
            try {
                params = new HashMap<String, Object>();
                params.put("function", "ClassLoader$NativeLibrary.init");
                params.put("path", libPath);
                params.put("realpath", libPath);
            } catch (Throwable t) {
                LogTool.traceHookWarn("system load hook check failed: " + t.getMessage(), t);
            }
            if (params != null) {
                HookHandler.doCheck(CheckParameter.Type.LOADLIBRARY, params);
            }
        }
    }
}
