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

import org.apache.log4j.Logger;
import org.mozilla.javascript.ScriptableObject;
import org.mozilla.javascript.annotations.JSFunction;


/**
 * JavaScript 环境中的 stdout 对象
 * 提供输出能力
 */
public class JSStdout extends ScriptableObject {
    private static final Logger LOGGER = Logger.getLogger(JSStdout.class.getPackage().getName() + ".log");

    public JSStdout() {
    }

    @Override
    public String getClassName() {
        return "Stdout";
    }

    @JSFunction
    public void write(Object message) {
        if (message instanceof String) {
            message = ((String) message).replaceAll("\n$", "");
        }
        LOGGER.info(message);
    }
}
