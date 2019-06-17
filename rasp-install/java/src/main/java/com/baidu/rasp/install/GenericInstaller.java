package com.baidu.rasp.install;

import com.baidu.rasp.RaspError;

import java.io.File;

/**
 * Created by tyy on 19-5-28.
 * 通用的 RASP 安装模块，不做启动脚本的修改
 */
public class GenericInstaller extends BaseStandardInstaller {

    public GenericInstaller(String serverName, String serverRoot) {
        super(serverName, serverRoot);
        // 关闭修改启动脚本的选项
        this.needModifyScript = false;
    }

    @Override
    protected String getInstallPath(String serverRoot) {
        return serverRoot + File.separator + "rasp";
    }

    @Override
    protected String getScript(String installPath) {
        return null;
    }

    @Override
    protected String modifyStartScript(String content) throws RaspError {
        return null;
    }
}
