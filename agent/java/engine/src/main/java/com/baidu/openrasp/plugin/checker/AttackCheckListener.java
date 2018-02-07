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

package com.baidu.openrasp.plugin.checker;

import com.baidu.openrasp.plugin.event.CheckEventListener;
import com.baidu.openrasp.plugin.info.AttackInfo;
import com.baidu.openrasp.plugin.info.EventInfo;

/**
 * Created by tyy on 17-11-22.
 *
 * 攻击检测事件监听器
 */
public class AttackCheckListener implements CheckEventListener {
    @Override
    public void onCheckUpdate(EventInfo info) {
        if (info instanceof AttackInfo) {
            Checker.ATTACK_ALARM_LOGGER.info(info);
        }
    }

}
