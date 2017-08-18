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

import com.fuxi.javaagent.config.Config;
import net.contentobjects.jnotify.JNotify;
import net.contentobjects.jnotify.JNotifyException;
import org.apache.commons.io.monitor.FileAlterationListener;
import org.apache.commons.io.monitor.FileAlterationObserver;

import java.io.File;

/**
 * Created by tyy on 4/17/17.
 * 文件事件的监视器，监视单位为文件夹
 */
public class FileScanMonitor {

    /**
     * 增加监视器
     *
     * @param path     监听的文件夹路径
     * @param listener 事件回调接口
     * @return 监听器的id
     * @throws JNotifyException {@link JNotifyException}
     */
    public static int addMonitor(String path, FileAlterationListener listener) throws JNotifyException {
        File file = new File(path);
        FileAlterationObserver observer = new FileAlterationObserver(file);
        observer.checkAndNotify();
        observer.addListener(listener);
        int mask = JNotify.FILE_CREATED | JNotify.FILE_DELETED
                | JNotify.FILE_MODIFIED;

        JNotify.init(Config.getConfig().getBaseDirectory());
        return JNotify.addWatch(path, mask, false, new FileEventListener(observer));
    }

    /**
     * 移除某一个文件夹监听
     *
     * @param watchId 增加文件夹监听的时候返回的监听器id
     */
    public static void removeMonitor(int watchId) {
        try {
            JNotify.removeWatch(watchId);
        } catch (JNotifyException e) {
            e.printStackTrace();
        }
    }


}
