package com.baidu.rasp.uninstall.windows;

import com.baidu.rasp.uninstall.BaseStandardUninstaller;

import java.util.Scanner;
import java.util.regex.Pattern;

/**
　　* @Description: jboss自动卸载
　　* @author anyang
　　* @date 2018/4/25 19:35
　　*/
public class Jboss4Uninstaller extends BaseStandardUninstaller {

    private static Pattern OPENRASP_REGEX = Pattern.compile(".*(\\s*OPENRASP\\s*|JAVA_OPTS.*\\\\rasp\\\\).*");

    public Jboss4Uninstaller(String serverName, String serverRoot) {
        super(serverName, serverRoot);
    }

    @Override
    protected String getInstallPath(String serverRoot) {
        return serverRoot + "\\rasp";
    }

    @Override
    protected String getScript(String installPath) {
        return installPath + "\\..\\bin\\run.bat";
    }

    @Override
    protected String recoverStartScript(String content) {

        StringBuilder sb = new StringBuilder();
        Scanner scanner = new Scanner(content);
        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            if (OPENRASP_REGEX.matcher(line).matches()) {
                continue;
            }
            sb.append(line).append(LINE_SEP);
        }
        return sb.toString();
    }
}
