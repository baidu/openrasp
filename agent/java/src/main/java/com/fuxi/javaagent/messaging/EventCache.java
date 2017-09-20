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

package com.fuxi.javaagent.messaging;

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
