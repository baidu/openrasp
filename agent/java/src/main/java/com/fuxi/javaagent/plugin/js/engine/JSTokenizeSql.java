/*
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

package com.fuxi.javaagent.plugin.js.engine;

import com.baidu.rasp.TokenGenerator;
import org.mozilla.javascript.BaseFunction;
import org.mozilla.javascript.Context;
import org.mozilla.javascript.Scriptable;

/**
 * Java 实现的 token 解析功能，将注册到 JS 中 RASP 对象上
 */
public class JSTokenizeSql extends BaseFunction {
    /**
     * @see BaseFunction#call(Context, Scriptable, Scriptable, Object[])
     * @param cx
     * @param scope
     * @param thisObj
     * @param args
     * @return
     */
    @Override
    public Object call(Context cx, Scriptable scope, Scriptable thisObj,
                       Object[] args) {
        if (args.length < 1) {
            return Context.getUndefinedValue();
        }
        if (!(args[0] instanceof String)) {
            return Context.getUndefinedValue();
        }
        String sql = (String) args[0];
        String[] result = TokenGenerator.tokenize(sql);
        int length = result.length;
        Scriptable array = cx.newArray(scope, length);
        for (int i = 0; i < length; i++) {
            array.put(i, array, result[i]);
        }
        return array;
    }

    /**
     * 提供获取该对象默认值的方法
     * console.log(thisObj) 即会输出此方法返回的值
     * @see Scriptable#getDefaultValue(Class)
     * @param hint
     * @return
     */
    @Override
    public Object getDefaultValue(Class<?> hint) {
        return "[Function: sql_tokenize]";
    }
}
