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

package com.baidu.openrasp.tool.filemonitor;

import org.apache.commons.io.monitor.FileAlterationListener;
import org.apache.commons.io.monitor.FileAlterationObserver;

import java.io.File;

/**
 * Created by tyy on 4/18/17.
 * 观察者扫描文件夹触发的文件变化事件的监听器
 * 屏蔽掉了文件夹事件
 */
public abstract class FileScanListener implements FileAlterationListener {

    /**
     * 观察者开始观察回调接口
     *
     * @param fileAlterationObserver 观察者对象
     */
    @Override
    public void onStart(FileAlterationObserver fileAlterationObserver) {

    }

    /**
     * 文件夹创建事件
     *
     * @param file 文件夹对象
     */
    @Override
    public void onDirectoryCreate(File file) {

    }

    /**
     * 文件夹变化事件
     *
     * @param file 文件夹事件
     */
    @Override
    public void onDirectoryChange(File file) {

    }

    /**
     * 文件夹删除事件
     *
     * @param file 文件夹对象
     */
    @Override
    public void onDirectoryDelete(File file) {

    }

    /**
     * 文件创建事件
     *
     * @param file 文件对象
     */
    public abstract void onFileCreate(File file);

    /**
     * 文件变化事件
     *
     * @param file 文件对象
     */
    public abstract void onFileChange(File file);

    /**
     * 文件删除事件
     *
     * @param file 文件对象
     */
    public abstract void onFileDelete(File file);

    /**
     * 观察者结束观察事件
     *
     * @param fileAlterationObserver 观察者对象
     */
    @Override
    public void onStop(FileAlterationObserver fileAlterationObserver) {

    }
}
