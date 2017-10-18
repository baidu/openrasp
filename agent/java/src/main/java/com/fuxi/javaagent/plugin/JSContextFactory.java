/**
 * Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
 * <p>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * <p>
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * <p>
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * <p>
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * <p>
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

package com.fuxi.javaagent.plugin;

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

            InputStream is;
            String name;
            String script;

            name = "shim.js";
            is = Object.class.getResourceAsStream("/" + name);
            script = IOUtils.toString(is, "UTF-8");
            cx.evaluateString(globalScope, script, name, 1, null);
            name = "console.js";
            is = Object.class.getResourceAsStream("/" + name);
            script = IOUtils.toString(is, "UTF-8");
            cx.evaluateString(globalScope, script, name, 1, null);
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
            RASP.defineProperty("set_rasp_config", new JSRASPConfig(), ScriptableObject.READONLY);
        } finally {
            JSContext.exit();
        }
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
