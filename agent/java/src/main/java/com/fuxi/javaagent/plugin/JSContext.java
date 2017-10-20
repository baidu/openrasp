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

import com.fuxi.javaagent.config.Config;
import com.fuxi.javaagent.plugin.event.AttackInfo;
import org.apache.log4j.Logger;
import org.mozilla.javascript.*;

import java.util.List;

public class JSContext extends Context {
    private static final Logger LOGGER = Logger.getLogger(JSContext.class.getPackage().getName() + ".log");

    private Scriptable scope = null;

    private List<List<CheckProcess>> checkPointList = null;

    private long pluginTime = Long.MIN_VALUE;

    private long timeout = Long.MAX_VALUE;

    public Scriptable getScope() {
        return scope;
    }

    public void setScope(Scriptable scope) {
        this.scope = scope;
    }

    public List<List<CheckProcess>> getCheckPointList() {
        return checkPointList;
    }

    public void setCheckPointList(List<List<CheckProcess>> checkPointList) {
        this.checkPointList = checkPointList;
    }

    public long getPluginTime() {
        return pluginTime;
    }

    public void setPluginTime(long pluginTime) {
        this.pluginTime = pluginTime;
    }

    public long getTimeout() {
        return timeout;
    }

    public void setTimeout(long timeout) {
        this.timeout = timeout;
    }

    public void clearTimeout() {
        timeout = Long.MAX_VALUE;
    }

    public boolean isTimeout() {
        return System.currentTimeMillis() > timeout;
    }

    public boolean check(CheckParameter parameter) {
        List<CheckProcess> processList = checkPointList.get(parameter.getType().ordinal());
        if (processList.size() < 1) {
            return false;
        }

        Scriptable params = (Scriptable) parameter.getParams();
        Scriptable requestContext = this.newObject(scope, "Context", new Object[]{parameter.getRequest()});

        Object[] functionArgs = {params, requestContext};
        Object tmp;
        CheckProcess checkProcess;
        Function function;
        ScriptableObject result;
        String action;
        String message;
        String name;
        int confidence;
        boolean block = false;

        setTimeout(System.currentTimeMillis() + Config.getConfig().getPluginTimeout());
        int size = processList.size();
        for (int i = 0; i < size; i++) {
            checkProcess = processList.get(i);
            function = checkProcess.getFunction();
            try {
                tmp = function.call(this, scope, function, functionArgs);
            } catch (RhinoException e) {
                LOGGER.info(e.details() + "\n" + e.getScriptStackTrace());
                if (isTimeout()) {
                    break;
                } else {
                    continue;
                }
            } catch (Exception e) {
                LOGGER.info(e);
                continue;
            }
            if (tmp == Context.getUndefinedValue()) {
                continue;
            }
            result = (ScriptableObject) tmp;
            tmp = result.get("action", result);
            if (!(tmp instanceof CharSequence)) {
                continue;
            }
            action = tmp.toString();
            if (action.equals("ignore")) {
                continue;
            }
            tmp = result.get("message", result);
            if (!(tmp instanceof CharSequence)) {
                tmp = "";
            }
            message = tmp.toString();
            tmp = result.get("name", result);
            if (!(tmp instanceof CharSequence)) {
                tmp = checkProcess.getPluginName();
            }
            name = tmp.toString();
            tmp = result.get("confidence", result);
            if (!(tmp instanceof Number)) {
                tmp = new Integer(0);
            }
            confidence = ((Number) tmp).intValue();
            CheckResult checkResult = new CheckResult(action, message, name, confidence);
            AttackInfo attackInfo = new AttackInfo(parameter, checkResult);
            PluginManager.ALARM_LOGGER.info(attackInfo.toString());
            block = block || action.equals("block");
        }
        return block;
    }
}
