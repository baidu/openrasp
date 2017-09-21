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

import com.eclipsesource.v8.V8Array;
import com.eclipsesource.v8.V8Object;
import com.eclipsesource.v8.utils.MemoryManager;
import com.eclipsesource.v8.utils.V8ObjectUtils;
import com.fuxi.javaagent.plugin.CheckParameter;
import com.fuxi.javaagent.plugin.CheckResult;

import java.util.LinkedList;
import java.util.List;

/**
 * Created by lanyuhang on 2017/7/17.
 */

/**
 * 请求检测任务
 */
public class JSCheckTask extends JSTask<List<CheckResult>> {
    private CheckParameter parameter;

    /**
     * (non-Javadoc)
     *
     * @param parameter 待检测参数
     */
    public JSCheckTask(CheckParameter parameter) {
        if (parameter == null) {
            throw new NullPointerException();
        }
        this.parameter = parameter;
    }

    /**
     * 线程池调用
     *
     * @return 检测结果列表
     * @throws Exception
     */
    @Override
    public List<CheckResult> call() throws Exception {
        LinkedList<CheckResult> results = new LinkedList<CheckResult>();
        jsEngine = localJSEngine.get();
        v8 = jsEngine.getV8();
        jsContext = jsEngine.getJsContext();
        MemoryManager scope = new MemoryManager(v8);
        try {
            String jsType = parameter.getType();
            V8Object jsParams = V8ObjectUtils.toV8Object(v8, parameter.getParams());
            jsContext.setJavaContext(parameter.getRequest());

            V8Array jsResults = jsEngine.check(jsType, jsParams, jsContext);
            CheckResult checkResult = null;
            for (int i = 0; i < jsResults.length(); i++) {
                V8Object jsResult = jsResults.getObject(i);
                int confidence = CheckResult.DEFAULT_CONFIDENCE_VALUE;
                try {
                    confidence = jsResult.getInteger("confidence");
                } catch (Exception e) {
                    //ignore
                }
                results.push(new CheckResult(
                        jsResult.getString("action"),
                        jsResult.getString("message"),
                        jsResult.getString("name"),
                        confidence
                ));
            }
        } finally {
            scope.release();
        }
        return results;
    }
}
