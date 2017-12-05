package com.baidu.rasp.install.linux;

import com.baidu.rasp.RaspError;
import com.baidu.rasp.install.BaseStandardInstaller;
import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;

import java.io.*;
import java.util.Scanner;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import static com.baidu.rasp.RaspError.E10001;

/**
 * Created by OpenRASP on 5/11/17.
 */
public class TomcatInstaller extends BaseStandardInstaller {

    private static String CATALINASHRASP = "JAVA_OPTS=\"-javaagent:${CATALINA_HOME}/rasp/rasp.jar ${JAVA_OPTS}\"";
    private static String CATALINASHLOG4J = "JAVA_OPTS=\"-Dlog4j.rasp.configuration=file://${CATALINA_HOME}/rasp/conf"
            + "/rasp-log4j.xml ${JAVA_OPTS}\"";

    TomcatInstaller(String serverName, String serverRoot) {
        super(serverName, serverRoot);
    }

    @Override
    protected String getInstallPath(String serverRoot) {
        String catalineBase;
        if (serverRoot == null) {
            String output = runCommand(new String[]{"bash", "-c", "ps -elf | grep catalina.base"});
            Pattern PATTERN = Pattern.compile("-Dcatalina\\.base=(\\S+)");
            Matcher m = PATTERN.matcher(output);
            catalineBase = (m.find() ? m.group(1) : null);
        } else {
            catalineBase = serverRoot;
        }
        return catalineBase + "/rasp";
    }

    @Override
    protected String getScript(String installDir) {
        return installDir + "/../bin/catalina.sh";
    }

    @Override
    protected String modifyStartScript(String content) throws RaspError {
        StringBuilder sb = new StringBuilder();
        Scanner scanner = new Scanner(content);
        int modifyConfigState = NOTFOUND;
        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            if (FOUND == modifyConfigState) {
                sb.append("  " + CATALINASHRASP + LINE_SEP);
                sb.append("  " + CATALINASHLOG4J + LINE_SEP);
                modifyConfigState = DONE;
            }
            if (DONE == modifyConfigState) {
                if (line.contains(CATALINASHRASP) || line.contains(CATALINASHLOG4J)) {
                    continue;
                }
            }
            if (!line.startsWith("#") && (line.contains("\"$1\" = \"start\"") || line.contains("\"$1\" = \"run\""))) {
                modifyConfigState = FOUND;
            }
            sb.append(line).append(LINE_SEP);
        }
        if (NOTFOUND == modifyConfigState) {
            throw new RaspError(E10001 + "[\"$1\" = \"start\"] or [\"$1\" = \"run\"]");
        }
        return sb.toString();
    }

}
