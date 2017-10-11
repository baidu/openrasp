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

import org.apache.log4j.Logger;
import org.mozilla.javascript.*;

import java.util.List;
import java.util.Map;

public class JSContext extends Context {
    private static final Logger LOGGER = Logger.getLogger(JSContext.class.getPackage().getName() + ".log");
    private static final Logger ALARM_LOGGER = Logger.getLogger(PluginManager.class.getPackage().getName() + ".alarm");

    public Scriptable scope = null;
    public List<List<CheckProcess>> checkPointList = null;
    public long startTime = 0;
    public long pluginTime = -1;

    public boolean check(CheckParameter parameter) {
        List<CheckProcess> processList = checkPointList.get(parameter.getType().ordinal());
        if (processList.size() < 1) {
            return false;
        }
        Map<String, Object> parameterParams = parameter.getParams();
        Scriptable parmas = newObject(scope);
        for(Map.Entry<String, Object> param : parameterParams.entrySet()) {
            Object value = param.getValue();
            if (value instanceof String) {
                parmas.put(param.getKey(), parmas, value);
            } else if (value instanceof List) {
                Scriptable array = newArray(scope, ((List) value).toArray());
                parmas.put(param.getKey(), parmas, array);
            }
        }

        Scriptable requestContext = this.newObject(scope, "Context", new Object[]{parameter.getRequest()});

        Object[] functionArgs = {parmas, requestContext};
        Object tmp;
        CheckProcess checkProcess;
        Function function;
        ScriptableObject result;
        String action;
        String message;
        String name;
        int confidence;
        boolean block = false;

        int size = processList.size();
        for (int i = 0; i < size; i++) {
            checkProcess = processList.get(i);
            function = checkProcess.getFunction();
            try {
                tmp = function.call(this, scope, function, functionArgs);
            } catch (RhinoException e) {
                LOGGER.info("\n" + e.details() + "\n" + e.getScriptStackTrace());
                continue;
            } catch (Exception e) {
                LOGGER.info(e);
                continue;
            }
            if(tmp == Context.getUndefinedValue()) {
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
            ALARM_LOGGER.info(attackInfo.toString());
            block = block || action.equals("block");
        }
        return block;
    }
}
