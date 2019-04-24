package com.baidu.openrasp.dependency;

import java.util.HashSet;

/**
 * @description: 依赖检查上报
 * @author: anyang
 * @create: 2019/04/19 16:07
 */
public class DependencyReport {
    private static final int REGISTER_DELAY = 60 * 1000;

    public DependencyReport() {
        Thread thread = new Thread(new DependencyThread());
        thread.setDaemon(true);
        thread.start();
    }

    class DependencyThread implements Runnable{
        @Override
        public void run() {
            while (true){
                HashSet<Dependency> dependencyHashSet=DependencyFinder.getDependencySet();
                for (Dependency dependency : dependencyHashSet){
                    System.out.println(dependency.name+"===="+dependency.version+"====="+dependency.location);
                }
                try {
                    Thread.sleep(REGISTER_DELAY);
                } catch (InterruptedException e) {
                    //continue next loop
                }
            }
        }
    }
}
