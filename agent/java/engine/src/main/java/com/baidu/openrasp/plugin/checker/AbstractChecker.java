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

import com.baidu.openrasp.plugin.event.CheckEventDispatcher;
import com.baidu.openrasp.plugin.event.CheckEventListener;
import com.baidu.openrasp.plugin.info.EventInfo;

import java.util.List;

/**
 * Created by tyy on 17-11-20.
 *
 * hook点参数检测接口
 */
public abstract class AbstractChecker implements Checker {

    private boolean canBlock = true;
    private CheckEventDispatcher eventDispatcher = new CheckEventDispatcher();

    public AbstractChecker() {
        this(true);
    }

    public AbstractChecker(boolean canBlock) {
        this.canBlock = canBlock;
    }

    @Override
    public boolean check(CheckParameter checkParameter) {
        List<EventInfo> eventInfos = checkParam(checkParameter);
        boolean isBlock = false;
        if (eventInfos != null) {
            for (EventInfo info : eventInfos) {
                if (info.isBlock()) {
                    isBlock = true;
                }
                dispatchCheckEvent(info);
            }
        }
        isBlock = isBlock && canBlock;
        return isBlock;
    }

    public void addCheckEventListener(CheckEventListener listener) {
        eventDispatcher.addCheckEventListener(listener);
    }

    public void dispatchCheckEvent(EventInfo info) {
        eventDispatcher.onCheckUpdate(info);
    }

    public boolean isCanBlock() {
        return canBlock;
    }

    public void setCanBlock(boolean canBlock) {
        this.canBlock = canBlock;
    }

    /**
     * 实现参数检测逻辑
     *
     * @param checkParameter 检测参数
     * @return 检测结果
     */
    public abstract List<EventInfo> checkParam(CheckParameter checkParameter);

}
