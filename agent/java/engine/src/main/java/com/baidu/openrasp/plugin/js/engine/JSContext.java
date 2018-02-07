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

import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.info.AttackInfo;
import com.baidu.openrasp.plugin.info.EventInfo;
import org.apache.log4j.Logger;
import org.mozilla.javascript.*;

import java.util.LinkedList;
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

    public List<EventInfo> check(CheckParameter parameter) {
        LinkedList<EventInfo> checkResults = new LinkedList<EventInfo>();
        List<CheckProcess> processList = checkPointList.get(parameter.getType().ordinal());
        if (processList == null || processList.size() < 1) {
            return null;
        }

        Object params = parameter.getParams();
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
            if (tmp == null || !(tmp instanceof NativeObject)) {
                continue;
            }
            result = (ScriptableObject) tmp;
            tmp = result.get("action");
            if (!(tmp instanceof CharSequence)) {
                continue;
            }
            action = tmp.toString();
            if (action.equals("ignore")) {
                continue;
            }
            tmp = result.get("message");
            if (tmp instanceof CharSequence) {
                message = tmp.toString();
            } else {
                message = "";
            }
            tmp = result.get("name");
            if (tmp instanceof CharSequence) {
                name = tmp.toString();
            } else {
                name = checkProcess.getPluginName();
            }
            tmp = result.get("confidence");
            if (tmp instanceof Number) {
                confidence = ((Number) tmp).intValue();
            } else {
                confidence = new Integer(0);
            }
            checkResults.add(new AttackInfo(parameter, action, message, name, confidence));
        }
        return checkResults;
    }
}
