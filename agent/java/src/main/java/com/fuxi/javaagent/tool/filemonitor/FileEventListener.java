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

import net.contentobjects.jnotify.JNotifyListener;
import org.apache.commons.io.monitor.FileAlterationObserver;

/**
 * Created by tyy on 6/7/17.
 * 可用于监听某一个文件夹事件的监听器
 * 使用系统事件作为驱动，实时性高
 * 将事件传递给观察者，由观察者扫描该文件夹来进一步确定事件事件具体类型
 */
public class FileEventListener implements JNotifyListener {

    private FileAlterationObserver observer;

    /**
     * constructor
     *
     * @param observer 关注某一个文件夹事件的观察者
     */
    public FileEventListener(FileAlterationObserver observer) {
        this.observer = observer;
    }

    /**
     * 文件重命名事件回调接口
     */
    @Override
    public void fileRenamed(int wd, String rootPath, String oldName,
                            String newName) {
        observer.checkAndNotify();
    }

    /**
     * 文件修改事件回调接口
     */
    @Override
    public void fileModified(int wd, String rootPath, String name) {
        observer.checkAndNotify();
    }

    /**
     * 文件文件删除事件回调接口
     */
    @Override
    public void fileDeleted(int wd, String rootPath, String name) {
        observer.checkAndNotify();
    }

    /**
     * 文件创建事件回调接口
     */
    @Override
    public void fileCreated(int wd, String rootPath, String name) {
        observer.checkAndNotify();
    }

}
