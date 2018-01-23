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

package com.baidu.openrasp.plugin.js.engine;

import com.baidu.openrasp.plugin.checker.CheckParameter;
import org.apache.commons.io.IOUtils;
import org.apache.log4j.Logger;
import org.mozilla.javascript.*;
import com.fuxi.javaagent.rhino.shim.*;

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

    private static JSContextFactory jsContextFactory = null;

    private ScriptableObject globalScope = null;
    private ScriptableObject RASP = null;
    private long pluginTime = 0;

    private JSContextFactory() throws Exception {
        ContextFactory.initGlobal(this);
        JSContext cx = (JSContext) JSContext.enter();
        cx.clearTimeout();
        try {
            globalScope = cx.initStandardObjects();

            globalScope.defineProperty("global", globalScope, ScriptableObject.READONLY);

            ScriptableObject.defineClass(globalScope, JSStdout.class);
            Object jsstdout = cx.newObject(globalScope, "Stdout");
            globalScope.defineProperty("stdout", jsstdout, ScriptableObject.READONLY);
            globalScope.defineProperty("stderr", jsstdout, ScriptableObject.READONLY);

            ScriptableObject.defineClass(globalScope, JSRequestContext.class);

            Script shim;
            shim = new Shim();
            shim.exec(cx, globalScope);
            shim = new Console();
            shim.exec(cx, globalScope);

            InputStream is;
            String name;
            String script;

            name = "error.js";
            is = Object.class.getResourceAsStream("/environment/" + name);
            script = IOUtils.toString(is, "UTF-8");
            cx.evaluateString(globalScope, script, name, 1, null);
            name = "checkpoint.js";
            is = Object.class.getResourceAsStream("/environment/" + name);
            script = IOUtils.toString(is, "UTF-8");
            cx.evaluateString(globalScope, script, name, 1, null);
            name = "rasp.js";
            is = Object.class.getResourceAsStream("/environment/" + name);
            script = IOUtils.toString(is, "UTF-8");
            cx.evaluateString(globalScope, script, name, 1, null);

            RASP = (ScriptableObject) ScriptableObject.getProperty(globalScope, "RASP");
            RASP.defineProperty("sql_tokenize", new JSTokenizeSql(), ScriptableObject.READONLY);
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
        } finally {
            JSContext.exit();
        }

        System.out.println("[OpenRASP] JSContextFactory Initialized");
    }

    public static void init() throws Exception {
        jsContextFactory = new JSContextFactory();
    }

    public static void setCheckScriptList(List<CheckScript> checkScriptList) {
        JSContext cx = (JSContext) JSContext.enter();
        cx.clearTimeout();
        try {
            ScriptableObject scope = (ScriptableObject) cx.newObject(jsContextFactory.globalScope);
            scope.setPrototype(jsContextFactory.globalScope);
            scope.setParentScope(null);
            Function clean = (Function) jsContextFactory.RASP.get("clean", jsContextFactory.RASP);
            clean.call(cx, scope, clean, null);
            for (CheckScript checkScript : checkScriptList) {
                cx.evaluateString(scope, "(function(){\n" + checkScript.getContent() + "\n})()", checkScript.getName(), 0, null);
            }
        } catch (Exception e) {
            LOGGER.info(e);
        } finally {
            jsContextFactory.pluginTime = System.currentTimeMillis();
            JSContext.exit();
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
            Scriptable scope = cx.newObject(jsContextFactory.globalScope);
            scope.setPrototype(jsContextFactory.globalScope);
            scope.setParentScope(null);

            NativeObject checkPoints = (NativeObject) jsContextFactory.RASP.get("checkPoints", jsContextFactory.RASP);
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
}
