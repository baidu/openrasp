package com.baidu.openrasp.cloud;

/**
 * Created by tyy on 19-5-17.
 *
 * 云控定时任务基类
 */
public abstract class CloudTimerTask implements Runnable {

    private int sleepTime;

    private boolean isAlive = true;

    public CloudTimerTask(int sleepTime) {
        this.sleepTime = sleepTime;
    }

    public void start() {
        Thread taskThread = new Thread(this);
        taskThread.setDaemon(true);
        taskThread.start();
    }

    public void stop() {
        this.isAlive = false;
    }

    public void run() {
        while (isAlive) {
            try {
                execute();
            } catch (Throwable t) {
                handleError(t);
            }
            try {
                // 和上面分开处理，避免心跳失败不走 sleep,不能放到 execute 之前，会导致第一次心跳不能马上运行
                Thread.sleep(sleepTime * 1000);
            } catch (InterruptedException e) {
                handleError(e);
            }
        }
    }

    abstract public void execute();

    abstract public void handleError(Throwable t);

    public void setAlive(boolean alive) {
        isAlive = alive;
    }
}
