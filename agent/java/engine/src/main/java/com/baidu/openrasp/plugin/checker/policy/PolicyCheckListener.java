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

package com.baidu.openrasp.plugin.checker.policy;

import com.baidu.openrasp.plugin.checker.Checker;
import com.baidu.openrasp.plugin.event.CheckEventListener;
import com.baidu.openrasp.plugin.info.EventInfo;
import com.baidu.openrasp.plugin.info.SecurityPolicyInfo;

/**
 * Created by tyy on 17-11-22.
 *
 * 基线检测事件监听器
 */
public class PolicyCheckListener implements CheckEventListener {

    @Override
    public void onCheckUpdate(EventInfo info) {
        if (info instanceof SecurityPolicyInfo) {
            Checker.POLICY_ALARM_LOGGER.info(info);
        }
    }

}
