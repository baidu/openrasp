/**
 * Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
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

package com.fuxi.javaagent.plugin.jsplugin;

import com.eclipsesource.v8.V8;
import com.eclipsesource.v8.V8Array;
import com.eclipsesource.v8.V8Object;
import com.eclipsesource.v8.V8ScriptCompilationException;
import com.eclipsesource.v8.utils.MemoryManager;
import com.fuxi.javaagent.config.Config;
import com.fuxi.javaagent.plugin.CheckScript;
import com.fuxi.javaagent.plugin.jsplugin.callbacks.JSContext;
import com.fuxi.javaagent.plugin.jsplugin.callbacks.JSStderr;
import com.fuxi.javaagent.plugin.jsplugin.callbacks.JSStdout;
import com.fuxi.javaagent.request.AbstractRequest;
import org.apache.commons.io.IOUtils;

import java.io.InputStream;
import java.util.List;
import org.apache.log4j.Logger;

/**
 * Created by lanyuhang on 2017/7/17.
 */

/**
 * 封装了V8
 *
 * 提供初始化，检测，终止等接口
 */
public class JSEngine {
    private static final Logger LOGGER = Logger.getLogger(JSEngine.class.getName());
    private Boolean isTerminated = false;
    private V8 v8;
    private V8Object RASP;
    private JSContext jsContext;

    public JSEngine() {
        v8 = V8.createV8Runtime("global", Config.getConfig().getBaseDirectory());
    }

    /**
     * 构造并初始化检测插件脚本
     *
     * @param scripts 检测插件脚本列表
     * @throws Exception
     */
    public JSEngine(List<CheckScript> scripts) throws Exception {
        this();
        if (scripts != null) {
            init(scripts);
        }
    }

    /**
     * 初始化插件检测脚本
     *
     * @param scripts 检测插件脚本列表
     * @throws Exception
     */
    public void init(List<CheckScript> scripts) throws Exception {
        MemoryManager scope = new MemoryManager(v8);

        V8Object stdout = new JSStdout(v8);
        V8Object stderr = new JSStderr(v8);
        v8.add("stdout", stdout);
        v8.add("stderr", stderr);

        // 加载初始化资源
        try {
            InputStream is = this.getClass().getResourceAsStream("/console.js");
            v8.executeScript(IOUtils.toString(is, "UTF-8"), "console.js", 0);
            is = this.getClass().getResourceAsStream("/environment/checkpoint.js");
            v8.executeScript(IOUtils.toString(is, "UTF-8"), "checkpoint.js", 0);
            is = this.getClass().getResourceAsStream("/environment/error.js");
            v8.executeScript(IOUtils.toString(is, "UTF-8"), "error.js", 0);
            is = this.getClass().getResourceAsStream("/environment/context.js");
            v8.executeScript(IOUtils.toString(is, "UTF-8"), "context.js", 0);
            is = this.getClass().getResourceAsStream("/environment/rasp.js");
            v8.executeScript(IOUtils.toString(is, "UTF-8"), "rasp.js", 0);
        } catch (Exception e) {
            scope.release();
            throw new Exception("load init scripts failed", e);
        }

        // 初始化检测脚本
        for (CheckScript script : scripts) {
            try {
                // 执行用户脚本，返回导出的初始化函数
                String name = script.getName();
                String content = script.getContent();
                v8.executeScript("(function(){" + content + "})()", name, 0);
            } catch (V8ScriptCompilationException e) {
                LOGGER.error("invalid script: " + script.getName(), e);
            } catch (Exception e) {
                LOGGER.error("exception thrown", e);
            }
        }
        scope.release();

        RASP = v8.getObject("RASP");
        jsContext = new JSContext(v8);
        LOGGER.info("new v8 created");
    }

    /**
     * 检测请求
     *
     * @param type    检测类型
     * @param params  检测参数
     * @param context 请求上下文
     * @return 检测结果数组
     * @throws Exception
     */
    public V8Array check(String type, V8Object params, V8Object context) throws Exception {
        synchronized (isTerminated) {
            isTerminated = false;
        }
        V8Array data = new V8Array(v8);
        data.push(type).push(params).push(context);
        V8Array results = null;
        try {
            results = RASP.executeArrayFunction("check", data);
        } catch (Exception e) {
            if (!isTerminated) {
                throw e;
            }
        } finally {
            data.release();
        }
        return results;
    }

    /**
     * 终止V8中正在执行的脚本
     */
    public void terminateExecution() {
        synchronized (isTerminated) {
            isTerminated = true;
        }
        v8.terminateExecution();
    }

    /**
     * 释放资源，防止内存泄漏
     */
    public synchronized void release() {
        if (RASP != null) {
            RASP.release();
            RASP = null;
        }
        if (jsContext != null) {
            jsContext.release();
            jsContext = null;
        }
        if (v8 != null) {
            v8.release();
            v8 = null;
        }
    }

    public V8 getV8() {
        return v8;
    }

    /**
     * 获取缓存的请求JSContext对象
     *
     * 调用{@link JSContext#setJavaContext(AbstractRequest)}设置回调访问的请求对象
     *
     * @return JSContext对象
     */
    public JSContext getJsContext() {
        return jsContext;
    }
}
