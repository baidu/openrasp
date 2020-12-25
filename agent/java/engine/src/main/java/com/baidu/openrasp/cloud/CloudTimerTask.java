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

package com.baidu.openrasp.cloud;

/**
 * Created by tyy on 19-5-17.
 *
 * 云控定时任务基类
 */
public abstract class CloudTimerTask implements Runnable {

    private Thread taskThread;

    private volatile boolean isAlive = true;

    private volatile boolean isSuspended = false;

    public CloudTimerTask(String name) {
        taskThread = new Thread(this);
        taskThread.setName(name);
        taskThread.setDaemon(true);
    }

    public void start() {
        if (!taskThread.isAlive()) {
            taskThread.start();
        }
    }

    public void interrupt() {
        taskThread.interrupt();
    }

    public void stop() {
        this.isAlive = false;
    }

    public void suspend() {
        this.isSuspended = true;
    }

    public void resume() {
        this.isSuspended = false;
    }

    public void run() {
        while (isAlive) {
            try {
                if (!isSuspended) {
                    try {
                        if (taskThread.isInterrupted()) {
                            Thread.interrupted();
                        }
                        execute();
                    } catch (Throwable t) {
                        handleError(t);
                    }
                }
                try {
                    // 和上面分开处理，避免心跳失败不走 sleep,不能放到 execute 之前，会导致第一次心跳不能马上运行
                    Thread.sleep(getSleepTime() * 1000);
                } catch (Throwable t) {
                    if (!(t instanceof InterruptedException)) {
                        handleError(t);
                    }
                }
            } catch (Throwable t) {
                System.out.println("OpenRASP cloud task failed: " + t.getMessage());
                t.printStackTrace();
            }
        }
    }

    abstract public long getSleepTime();

    abstract public void execute();

    abstract public void handleError(Throwable t);

    public void setAlive(boolean alive) {
        isAlive = alive;
    }
}
