/*
 * Copyright 2017-2019 Baidu Inc.
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

package com.baidu.rasp.uninstall;

import com.baidu.rasp.App;
import com.baidu.rasp.Attacher;
import com.baidu.rasp.RaspError;
import com.baidu.rasp.install.BaseStandardInstaller;

import java.io.File;
import java.io.IOException;
import java.util.Scanner;

import static com.baidu.rasp.RaspError.E10002;
import static com.baidu.rasp.RaspError.E10003;

/**
 * @author anyang
 * @Description: 自动卸载基础类
 * @date 2018/4/25 19:37
 */
public abstract class BaseStandardUninstaller implements Uninstaller {
    private String serverRoot;
    private String serverName;
    public static String LINE_SEP = System.getProperty("line.separator");

    public BaseStandardUninstaller(String serverName, String serverRoot) {
        this.serverRoot = serverRoot;
        this.serverName = serverName;
    }

    @Override
    public void uninstall() throws RaspError, IOException {
        File installDir = new File(getInstallPath(serverRoot));
        BaseStandardInstaller.modifyFolerPermission(installDir.getCanonicalPath());
        if (App.isAttach) {
            System.out.println("Attach the rasp to process with pid " + App.pid);
            new Attacher(App.pid + "", App.baseDir).doAttach(Attacher.MODE_UNINSTALL);
        }
        // 删除文件
        delRaspFolder(getInstallPath(serverRoot));
        // 找到要修改的启动脚本
        File script = new File(getScript(installDir.getPath()));
        if (!script.exists()) {
            throw new RaspError(E10003 + script.getAbsolutePath());
        }

        // 还原安装时修改的脚本
        System.out.println("Updating startup script\n- " + script.getCanonicalPath());
        String original = BaseStandardInstaller.read(script);
        String modified = recoverStartScript(original);
        BaseStandardInstaller.write(script, modified);

        if (!App.isAttach) {
            System.out.println("\nUninstallation completed without errors.\nPlease restart application server to take effect.");
        }
    }

    public void delAllFile(String path) throws RaspError {
        File file = new File(path);
        String[] tempList = file.list();
        File temp = null;
        for (String s : tempList) {
            if (path.endsWith(File.separator)) {
                temp = new File(path + s);
            } else {
                temp = new File(path + File.separator + s);
            }
            if (temp.isFile()) {
                temp.delete();
            }
            if (temp.isDirectory()) {
                // 先删除文件夹里面的文件
                delAllFile(path + File.separator + s);
                // 再删除空文件夹
                delRaspFolder(path + File.separator + s);
            }
        }
    }

    public void delRaspFolder(String folderPath) throws RaspError {

        File folder = new File(folderPath);
        if (!folder.exists()) {
            throw new RaspError(E10002 + folderPath);
        }
        // 删除完rasp文件夹里面所有文件和子文件夹
        delAllFile(folderPath);
        // 删除rasp空文件夹
        folder.delete();

    }

    //判断tomcat的版本是否大于8
    protected boolean checkTomcatVersion() {
        String javaVersion = System.getProperty("java.version");
        return javaVersion != null && (javaVersion.startsWith("1.9") || javaVersion.startsWith("10.")
                || javaVersion.startsWith("11."));
    }

    protected String getUninstallContent(String content) {
        StringBuilder sb = new StringBuilder();
        boolean isDelete = false;
        Scanner scanner = new Scanner(content);
        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            if (line.contains("BEGIN OPENRASP")) {
                isDelete = true;
                continue;
            }
            if (line.contains("END OPENRASP")) {
                isDelete = false;
                continue;
            }
            if (!isDelete) {
                sb.append(line).append(LINE_SEP);
            }
        }
        return sb.toString();
    }

    protected abstract String getInstallPath(String serverRoot);

    protected abstract String getScript(String installPath);

    protected abstract String recoverStartScript(String content) throws RaspError;
}
