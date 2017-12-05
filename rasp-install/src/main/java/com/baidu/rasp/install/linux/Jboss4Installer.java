package com.baidu.rasp.install.linux;

import com.baidu.rasp.RaspError;
import com.baidu.rasp.install.BaseStandardInstaller;

import java.util.Scanner;

import static com.baidu.rasp.RaspError.E10001;

/**
 * Created by OpenRASP on 5/15/17.
 */
public class Jboss4Installer extends BaseStandardInstaller {

    private static String RUNSHRASP = "JAVA_OPTS=\"-javaagent:${JBOSS_HOME}/rasp/rasp.jar ${JAVA_OPTS}\"";
    private static String RUNSHLOG4J = "JAVA_OPTS=\"-Dlog4j.rasp.configuration="
            + "file://${JBOSS_HOME}/rasp/conf/rasp-log4j.xml ${JAVA_OPTS}\"";

    Jboss4Installer(String serverName, String serverRoot) {
        super(serverName, serverRoot);
    }

    @Override
    protected String getInstallPath(String serverRoot) {
        return serverRoot + "/rasp";
    }

    @Override
    protected String getScript(String installPath) {
        return installPath + "/../bin/run.sh";
    }

    @Override
    protected String modifyStartScript(String content) throws RaspError {
        int modifyConfigState = NOTFOUND;
        StringBuilder sb = new StringBuilder();
        Scanner scanner = new Scanner(content);
        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            if (FOUND == modifyConfigState) {
                sb.append("  " + RUNSHRASP + LINE_SEP);
                sb.append("  " + RUNSHLOG4J + LINE_SEP);
                modifyConfigState = DONE;
            }
            if (DONE == modifyConfigState) {
                if (line.contains(RUNSHRASP) || line.contains(RUNSHLOG4J)) {
                    continue;
                }
            }
            if (line.startsWith("JAVA_OPTS=") && NOTFOUND == modifyConfigState) {
                modifyConfigState = FOUND;
            }
            if (line.contains(RUNSHRASP) || line.contains(RUNSHLOG4J)) {
                continue;
            }
            sb.append(line).append(LINE_SEP);
        }
        if (NOTFOUND == modifyConfigState) {
            throw new RaspError(E10001 + "\"JAVA_OPTS=\"");
        }
        return sb.toString();
    }

}
