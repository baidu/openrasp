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

package com.baidu.openrasp.messaging;

import org.apache.log4j.spi.LoggingEvent;

import java.util.Iterator;
import java.util.LinkedList;

/**
 * Created by lxk on 9/12/17.
 */
public class EventCache {

    static int DEFAULT_MAX_SIZE = 256;
    private static int DEFAULT_SIZE_PER_EVENT = 512;

    private int size;
    private LinkedList<LoggingEvent> list = null;

    public EventCache() {
        size = DEFAULT_MAX_SIZE;
        list = new LinkedList<LoggingEvent>();
    }

    public boolean addEvent(LoggingEvent event) {
        synchronized (list) {
            list.add(event);
            if (list.size() >= size) {
                return true;
            }
            return false;
        }
    }

    public String getJsonBody() {
        synchronized (list) {
            StringBuilder sb = new StringBuilder(list.size() * DEFAULT_SIZE_PER_EVENT);
            sb.append("{\"data\":[");
            Iterator<LoggingEvent> iterator = list.iterator();
            while (iterator.hasNext()) {
                sb.append(iterator.next().getMessage().toString());
                sb.append(",");
            }
            sb.deleteCharAt(sb.length() - 1);
            sb.append("]}");
            return sb.toString();
        }
    }

    public boolean isEmpty() {
        synchronized (list) {
            return list.isEmpty();
        }
    }

    public void clear() {
        synchronized (list) {
            list.clear();
        }
    }

    public void setSize(int size) {
        synchronized (list) {
            if (size < 0 || size > 1024) {
                return;
            } else {
                this.size = size;
            }
        }

    }
}
