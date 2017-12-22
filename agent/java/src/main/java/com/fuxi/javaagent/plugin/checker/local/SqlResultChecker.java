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

package com.fuxi.javaagent.plugin.checker.local;

import com.fuxi.javaagent.config.Config;
import com.fuxi.javaagent.plugin.checker.AttackChecker;
import com.fuxi.javaagent.plugin.checker.CheckParameter;
import com.fuxi.javaagent.plugin.info.AttackInfo;
import com.fuxi.javaagent.plugin.info.EventInfo;

import java.util.LinkedList;
import java.util.List;

/**
 * Created by tyy on 17-11-20.
 *
 * sql查询结果检测
 */
public class SqlResultChecker extends AttackChecker {

    public SqlResultChecker() {
        super();
    }

    public SqlResultChecker(boolean canBlock) {
        super(canBlock);
    }

    @Override
    public List<EventInfo> checkParam(CheckParameter checkParameter) {
        LinkedList<EventInfo> result = new LinkedList<EventInfo>();
        int queryCount = (Integer) checkParameter.getParam("query_count");
        int slowQueryMinCount = Config.getConfig().getSqlSlowQueryMinCount();
        if (queryCount == slowQueryMinCount + 1) {
            result.add(AttackInfo.createLocalAttackInfo(checkParameter, EventInfo.CHECK_ACTION_INFO, "慢查询: 使用SELECT语句读取了超过" + slowQueryMinCount + "行数据"));
        }
        return result;
    }
}
