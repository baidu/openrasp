package com.baidu.rasp.uninstall.windows;

import com.baidu.rasp.uninstall.BaseStandardUninstaller;
import com.baidu.rasp.install.windows.ResinInstaller;

import java.io.File;
import java.util.Scanner;
import java.util.regex.Pattern;

/**
　　* @Description:resin自动卸载
　　* @author anyang
　　* @date 2018/4/25 19:36
　　*/
public class ResinUninstaller extends BaseStandardUninstaller{

    private static Pattern OPENRASP_REGEX_WINDOWS = Pattern.compile(".*(\\s*OPENRASP\\s*|\\s*<jvm-arg>.*\\\\rasp\\\\).*");
    private static Pattern OPENRASP_REGEX_LINUX = Pattern.compile(".*(\\s*OPENRASP\\s*|\\s*<jvm-arg>.*/rasp/).*");

    public ResinUninstaller(String serverName,String serverRoot) {
        super(serverName,serverRoot);
    }

    @Override
    protected String getInstallPath(String serverRoot) {
        return serverRoot + File.separator+"rasp";
    }

    @Override
    protected String getScript(String installPath) {

        if (ResinInstaller.getVersion(installPath)==3){

            return new File(installPath).getParent()+File.separator+"conf"+File.separator+"resin.conf";

        }else {

            return new File(installPath).getParent()+File.separator+"conf"+File.separator+"cluster-default.xml";
        }
    }

    @Override
    protected String recoverStartScript(String content) {

        StringBuilder sb = new StringBuilder();
        Scanner scanner = new Scanner(content);
        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            if (OPENRASP_REGEX_WINDOWS.matcher(line).matches()||OPENRASP_REGEX_LINUX.matcher(line).matches()) {
                continue;
            }
            sb.append(line).append(LINE_SEP);
        }

        return sb.toString();
    }
}
