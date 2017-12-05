/*
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

package com.fuxi.javaagent.plugin.checker;

import com.fuxi.javaagent.plugin.event.CheckEventDispatcher;
import com.fuxi.javaagent.plugin.event.CheckEventListener;
import com.fuxi.javaagent.plugin.info.EventInfo;

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
