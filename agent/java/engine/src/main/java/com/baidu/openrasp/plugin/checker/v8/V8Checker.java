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

package com.baidu.openrasp.plugin.checker.v8;

import com.baidu.openrasp.plugin.checker.AttackChecker;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.info.EventInfo;
import com.baidu.openrasp.plugin.js.JS;


import java.util.List;

/**
 * Created by tyy on 17-11-20.
 *
 * 使用 js 插件检测
 */
public class V8Checker extends AttackChecker {

    public V8Checker() {
        super();
    }

    public V8Checker(boolean canBlock) {
        super(canBlock);
    }

    /**
     * 执行js插件进行安全检测
     *
     * @param checkParameter 检测参数 {@link CheckParameter}
     * @return 检测结果
     */
    @Override
    public List<EventInfo> checkParam(CheckParameter checkParameter) {
        return JS.Check(checkParameter);
    }
}
