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

package com.fuxi.javaagent.tool.filemonitor;

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
