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

package com.baidu.openrasp.plugin.checker.local;

import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.plugin.checker.AttackChecker;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.info.AttackInfo;
import com.baidu.openrasp.plugin.info.EventInfo;

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
        Integer queryCount = (Integer) checkParameter.getParam("query_count");
        if (queryCount != null) {
            int slowQueryMinCount = Config.getConfig().getSqlSlowQueryMinCount();
            if (queryCount == slowQueryMinCount) {
                result.add(AttackInfo.createLocalAttackInfo(checkParameter, EventInfo.CHECK_ACTION_INFO, "慢查询: 使用SELECT语句读取了大于等于" + slowQueryMinCount + "条数据"));
            }
        }
        return result;
    }
}
