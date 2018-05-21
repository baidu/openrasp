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

package com.baidu.openrasp.plugin.event;

import com.baidu.openrasp.plugin.info.EventInfo;

import java.util.LinkedList;

/**
 * Created by tyy on 17-11-21.
 *
 * 插件事件派发
 */
public class CheckEventDispatcher implements CheckEventListener {

    private LinkedList<CheckEventListener> listeners = new LinkedList<CheckEventListener>();

    public void addCheckEventListener(CheckEventListener listener) {
        listeners.add(listener);
    }

    @Override
    public void onCheckUpdate(EventInfo eventInfo) {
        for (CheckEventListener listener : listeners) {
            listener.onCheckUpdate(eventInfo);
        }
    }
}
