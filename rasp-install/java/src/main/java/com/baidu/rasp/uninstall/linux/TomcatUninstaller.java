package com.baidu.rasp.uninstall.linux;

import com.baidu.rasp.uninstall.BaseStandardUninstaller;

import java.io.File;
import java.util.Scanner;
import java.util.regex.Pattern;

/**
　　* @Description: tomcat自动卸载
　　* @author anyang
　　* @date 2018/4/25 19:34
　　*/
public class TomcatUninstaller extends BaseStandardUninstaller {

    private static Pattern OPENRASP_REGEX = Pattern.compile(".*(\\s*OPENRASP\\s*|JAVA_OPTS.*/rasp/).*");

    public TomcatUninstaller(String serverName, String serverRoot) {
        super(serverName, serverRoot);
    }
    @Override
    protected String getInstallPath(String serverRoot) {
        return serverRoot + "/rasp/";
    }

    @Override
    protected String getScript(String installPath) {

        return new File(installPath).getParent() + File.separator + "bin"+File.separator+"catalina.sh";
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
