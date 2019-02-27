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

import com.baidu.openrasp.plugin.antlr.CmdTokenizeErrorListener;
import com.baidu.openrasp.plugin.antlr.TokenGenerator;
import com.baidu.openrasp.plugin.antlr.TokenResult;
import org.mozilla.javascript.BaseFunction;
import org.mozilla.javascript.Context;
import org.mozilla.javascript.Scriptable;

import java.util.ArrayList;

/**
 * Java 实现的 token 解析功能，将注册到 JS 中 RASP 对象上
 */
public class JSTokenizeCmd extends BaseFunction {
    private static CmdTokenizeErrorListener tokenizeErrorListener = new CmdTokenizeErrorListener();

    /**
     * @param cx
     * @param scope
     * @param thisObj
     * @param args
     * @return
     * @see BaseFunction#call(Context, Scriptable, Scriptable, Object[])
     */
    @Override
    public Object call(Context cx, Scriptable scope, Scriptable thisObj, Object[] args) {
        if (args.length < 1) {
            return Context.getUndefinedValue();
        }
        if (!(args[0] instanceof String)) {
            return Context.getUndefinedValue();
        }
        String cmd = (String) args[0];
        ArrayList<TokenResult> result = TokenGenerator.cmdTokenize(cmd, tokenizeErrorListener);
        int length = result.size();
        Scriptable array = cx.newArray(scope, length);
        for (int i = 0; i < length; i++) {
            Scriptable node = cx.newObject(scope);
            node.put("text", node, result.get(i).getText());
            node.put("start", node, result.get(i).getStart());
            node.put("stop", node, result.get(i).getStop());
            array.put(i, array, node);
        }
        return array;
    }

    /**
     * 提供获取该对象默认值的方法
     * console.log(thisObj) 即会输出此方法返回的值
     *
     * @param hint
     * @return
     * @see Scriptable#getDefaultValue(Class)
     */
    @Override
    public Object getDefaultValue(Class<?> hint) {
        return "[Function: cmd_tokenize]";
    }
}
