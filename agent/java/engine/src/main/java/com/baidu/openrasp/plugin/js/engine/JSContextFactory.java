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

package com.baidu.openrasp.plugin.js.engine;

import com.baidu.openrasp.EngineBoot;
import com.baidu.openrasp.cloud.model.CloudCacheModel;
import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.fuxi.javaagent.rhino.shim.Console;
import com.fuxi.javaagent.rhino.shim.Shim;
import com.google.gson.Gson;
import org.apache.commons.io.IOUtils;
import org.apache.log4j.Logger;
import org.mozilla.javascript.*;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

/**
 * Created by lanyuhang on 2017/9/25.
 */

/**
 * Rhino JSContext 构造工厂类
 */
public class JSContextFactory extends ContextFactory {
    private static final Logger LOGGER = Logger.getLogger(JSContextFactory.class.getPackage().getName() + ".log");
    private static final String PLUGIN_NAME = "official.js";

    private static JSContextFactory jsContextFactory = null;

    private static ScriptableObject globalScope = null;
    private static ScriptableObject RASP = null;
    private long pluginTime = 0;

    private JSContextFactory() throws Exception {
        ContextFactory.initGlobal(this);
        JSContext cx = (JSContext) JSContext.enter();
        cx.clearTimeout();
        try {
            globalScope = cx.initStandardObjects();
            RASP = perpareLoadPlugin(cx, globalScope);
        } finally {
            JSContext.exit();
        }
        System.out.println("[OpenRASP] JSContextFactory Initialized");
    }

    public static void init() throws Exception {
        jsContextFactory = new JSContextFactory();
    }

    public static void release() {
        if (CloudUtils.checkCloudControlEnter()) {
            setCloudCheckScript(null, null, null, null);
        } else {
            setCheckScriptList(null);
        }
        jsContextFactory = null;
    }

    public static void setCheckScriptList(List<CheckScript> checkScriptList) {
        if (jsContextFactory != null) {
            JSContext cx = (JSContext) JSContext.enter();
            cx.clearTimeout();
            try {
                ScriptableObject scope = (ScriptableObject) cx.newObject(globalScope);
                scope.setPrototype(globalScope);
                scope.setParentScope(null);
                Function clean = (Function) RASP.get("clean", RASP);
                clean.call(cx, scope, clean, null);
                if (checkScriptList != null) {
                    for (CheckScript checkScript : checkScriptList) {
                        cx.evaluateString(scope, "(function(){\n" + checkScript.getContent() + "\n})()", checkScript.getName(), 0, null);
                    }
                }
                //插件更新成功后，清空全局的LRU缓存
                Config.commonLRUCache.clear();
                //插件更新成功后，设置algorithmConfig
                algorithmConfigSet();
            } catch (Exception e) {
                String message = "new plugin update failed";
                int errorCode = ErrorType.PLUGIN_ERROR.getCode();
                LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
            } finally {
                jsContextFactory.pluginTime = System.currentTimeMillis();
                JSContext.exit();
            }
        }
    }

    public static void setCloudCheckScript(String plugin, String md5, String version, Long deliveryTime) {
        if (jsContextFactory != null) {
            try {
                JSContext cx = (JSContext) JSContext.enter();
                cx.clearTimeout();
                ScriptableObject global = cx.initStandardObjects();
                ScriptableObject tempRASP = perpareLoadPlugin(cx, global);
                ScriptableObject tempScope = (ScriptableObject) cx.newObject(global);
                tempScope.setPrototype(global);
                tempScope.setParentScope(null);
                if (plugin != null) {
                    cx.evaluateString(tempScope, "(function(){\n" + plugin + "\n})()", PLUGIN_NAME, 0, null);
                }
                CloudCacheModel.getInstance().setPlugin(plugin);
                CloudCacheModel.getInstance().setPluginVersion(version);
                CloudCacheModel.getInstance().setPluginMD5(md5);
                CloudCacheModel.getInstance().setConfigTime(deliveryTime);
                globalScope = global;
                RASP = tempRASP;
                //插件更新成功后，清空全局的LRU缓存
                Config.commonLRUCache.clear();
                //插件更新成功后，设置algorithmConfig
                algorithmConfigSet();
            } catch (Throwable e) {
                String message = "new plugin update failed";
                int errorCode = ErrorType.PLUGIN_ERROR.getCode();
                LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
            } finally {
                jsContextFactory.pluginTime = System.currentTimeMillis();
                JSContext.exit();
            }
        }
    }

    /**
     * 获取当前线程绑定的 Context
     * 当该方法创建新 Context 时，初始化 global scope 并保存在 Context 的 ThreadLocal 中
     * 重复调用该方法不会增加 Context 中的引用计数，也不需要调用 Exit 解除绑定和释放
     * 该方法创建的 Context 与 Thread 同时释放
     *
     * @return 与当前线程绑定的 Context
     */
    public static JSContext enterAndInitContext() {
        JSContext cx = (JSContext) JSContext.getCurrentContext();
        if (cx == null) {
            cx = (JSContext) jsContextFactory.enterContext();
        }
        if (cx.getPluginTime() < jsContextFactory.pluginTime) {
            cx.setPluginTime(System.currentTimeMillis());
            Scriptable scope = cx.newObject(globalScope);
            scope.setPrototype(globalScope);
            scope.setParentScope(null);

            NativeObject checkPoints = (NativeObject) RASP.get("checkPoints", RASP);
            List<List<CheckProcess>> checkPointList = new ArrayList<List<CheckProcess>>(CheckParameter.Type.values().length);
            for (int i = 0; i < CheckParameter.Type.values().length; i++) {
                NativeArray functions = (NativeArray) checkPoints.get(CheckParameter.Type.values()[i].toString());
                if (functions == null) {
                    checkPointList.add(null);
                    continue;
                }
                List<CheckProcess> functionList = new ArrayList<CheckProcess>(functions.size());
                for (int j = 0; j < functions.size(); j++) {
                    NativeObject functionObj = (NativeObject) functions.get(j);
                    Function function = (Function) functionObj.get("func");
                    String pluginName = (String) ((NativeObject) functionObj.get("plugin")).get("name");
                    functionList.add(new CheckProcess(function, pluginName));
                }
                checkPointList.add(functionList);
            }

            cx.setScope(scope);
            cx.setCheckPointList(checkPointList);
        }

        return cx;
    }

    /**
     * 创建 JSContext 并设置默认值
     *
     * @return
     */
    @Override
    protected JSContext makeContext() {
        JSContext cx = new JSContext();
        // 每10 * 1000 * 1000个指令中断一次，用于检测超时
        cx.setInstructionObserverThreshold(10 * 1000 * 1000);
        cx.setLanguageVersion(Context.VERSION_ES6);
        // 使用解释执行
        cx.setOptimizationLevel(9);
        return cx;
    }

    @Override
    public boolean hasFeature(Context cx, int featureIndex) {
        switch (featureIndex) {
            case Context.FEATURE_MEMBER_EXPR_AS_FUNCTION_NAME:
                return true;

            case Context.FEATURE_RESERVED_KEYWORD_AS_IDENTIFIER:
                return true;

            case Context.FEATURE_PARENT_PROTO_PROPERTIES:
                return false;

            case Context.FEATURE_LOCATION_INFORMATION_IN_ERROR:
                return true;

            case Context.FEATURE_STRICT_MODE:
                return true;
        }
        return super.hasFeature(cx, featureIndex);
    }

    /**
     * 每10 * 1000 * 1000个指令触发一次，用于检测超时
     * 判断超时后抛出异常终止引擎执行
     *
     * @param cx
     * @param instructionCount
     * @see JSContextFactory#makeContext()
     */
    @Override
    protected void observeInstructionCount(Context cx, int instructionCount) {
        JSContext jscx = (JSContext) cx;
        if (jscx.isTimeout()) {
            JSContext.reportError("Error: Plugin Execution Timeout");
        }
    }

    /**
     * 初始化globalScope和RASP
     */
    private static ScriptableObject perpareLoadPlugin(JSContext cx, ScriptableObject scope) throws Exception {

        scope.defineProperty("global", scope, ScriptableObject.READONLY);
        ScriptableObject.defineClass(scope, JSStdout.class);
        Object jsstdout = cx.newObject(scope, "Stdout");
        scope.defineProperty("stdout", jsstdout, ScriptableObject.READONLY);
        scope.defineProperty("stderr", jsstdout, ScriptableObject.READONLY);
        ScriptableObject.defineClass(scope, JSRequestContext.class);
        Script shim;
        shim = new Shim();
        shim.exec(cx, scope);
        shim = new Console();
        shim.exec(cx, scope);

        InputStream is;
        String name;
        String script;

        name = "error.js";
        is = EngineBoot.class.getResourceAsStream("/environment/" + name);
        script = IOUtils.toString(is, "UTF-8");
        cx.evaluateString(scope, script, name, 1, null);
        name = "checkpoint.js";
        is = EngineBoot.class.getResourceAsStream("/environment/" + name);
        script = IOUtils.toString(is, "UTF-8");
        cx.evaluateString(scope, script, name, 1, null);
        name = "rasp.js";
        is = EngineBoot.class.getResourceAsStream("/environment/" + name);
        script = IOUtils.toString(is, "UTF-8");
        cx.evaluateString(scope, script, name, 1, null);
        ScriptableObject RASP = (ScriptableObject) ScriptableObject.getProperty(scope, "RASP");
        RASP.defineProperty("sql_tokenize", new JSTokenizeSql(), ScriptableObject.READONLY);
        RASP.defineProperty("cmd_tokenize", new JSTokenizeCmd(), ScriptableObject.READONLY);
        RASP.defineProperty("config_set", new JSRASPConfig(), ScriptableObject.READONLY);
        RASP.defineProperty("get_jsengine", new BaseFunction() {
            @Override
            public Object call(Context cx, Scriptable scope, Scriptable thisObj, Object[] args) {
                return "rhino";
            }

            @Override
            public Object getDefaultValue(Class<?> hint) {
                return "[Function: get_jsengine]";
            }
        }, ScriptableObject.READONLY);
        return RASP;
    }

    /**
     * 插件更新成功后，设置algorithmConfig
     */
    private static void algorithmConfigSet() {
        if (RASP != null) {
            Object config = RASP.get("algorithmConfig");
            if (config != null) {
                Config.getConfig().setAlgorithmConfig(new Gson().toJson(config));
            }
        }
    }
}
