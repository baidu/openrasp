package com.baidu.rasp.install.windows;

import com.baidu.rasp.RaspError;
import com.baidu.rasp.install.BaseStandardInstaller;

import java.util.Scanner;

import static com.baidu.rasp.RaspError.E10001;

/**
 * Created by OpenRASP on 5/15/17.
 */
public class Jboss4Installer extends BaseStandardInstaller {

    private static String RUNSHRASP = "set JAVA_OPTS=-javaagent:%JBOSS_HOME%\\rasp\\rasp.jar %JAVA_OPTS%";
    private static String RUNSHLOG4J = "set JAVA_OPTS=-Dlog4j.rasp.configuration="
            + "file:%JBOSS_HOME%\\rasp\\conf\\rasp-log4j.xml %JAVA_OPTS%";

    Jboss4Installer(String serverName, String serverRoot) {
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
    protected String modifyStartScript(String content) throws RaspError {
        int modifyConfigState = NOTFOUND;
        StringBuilder sb = new StringBuilder();
        Scanner scanner = new Scanner(content);
        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            if (FOUND == modifyConfigState) {
                sb.append(RUNSHRASP + LINE_SEP);
                sb.append(RUNSHLOG4J + LINE_SEP);
                modifyConfigState = DONE;
            }
            if (DONE == modifyConfigState) {
                if (line.contains(RUNSHRASP) || line.contains(RUNSHLOG4J)) {
                    continue;
                }
            }
            if (line.startsWith("rem Setup JBoss specific properties") && NOTFOUND == modifyConfigState) {
                modifyConfigState = FOUND;
            }
            if (line.contains(RUNSHRASP) || line.contains(RUNSHLOG4J)) {
                continue;
            }
            sb.append(line).append(LINE_SEP);
        }
        if (NOTFOUND == modifyConfigState) {
            throw new RaspError(E10001 + "\"rem Setup JBoss specific properties\"");
        }
        return sb.toString();
    }

}
