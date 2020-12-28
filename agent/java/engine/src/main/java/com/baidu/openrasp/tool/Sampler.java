/*
 * Copyright 2017-2021 Baidu Inc.
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

package com.baidu.openrasp.tool;

import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class Sampler {

    private Lock lock = new ReentrantLock();
    private long nextClearTime = 0;
    private int count = 0;
    private int interval;
    private int burst;

    public Sampler() {
        update(0, 0);
    }

    public Sampler(int interval, int burst) {
        update(interval, burst);
    }

    public void update(int interval, int burst) {
        if (this.interval != interval || this.burst != burst) {
            lock.lock();
            if (this.interval != interval || this.burst != burst) {
                this.interval = interval;
                this.burst = burst;
                this.nextClearTime = 0;
                this.count = 0;
            }
            lock.unlock();
        }
    }

    public boolean check() {
        if (interval == 0 || burst == 0) {
            return false;
        }
        long time = System.currentTimeMillis() / 1000;
        if (time >= nextClearTime) {
            lock.lock();
            if (time >= nextClearTime) {
                count = 0;
                nextClearTime = time + interval;
                nextClearTime -= nextClearTime % interval;
            }
            lock.unlock();
        }
        if (count >= burst) {
            return false;
        }
        lock.lock();
        count++;
        lock.unlock();
        return true;
    }

}