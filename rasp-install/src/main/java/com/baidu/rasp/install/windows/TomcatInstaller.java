package com.baidu.rasp.install.windows;

import com.baidu.rasp.RaspError;
import com.baidu.rasp.install.BaseStandardInstaller;
import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.util.Scanner;

import static com.baidu.rasp.RaspError.E10001;

/**
 * Created by OpenRASP on 5/15/17.
 */
public class TomcatInstaller extends BaseStandardInstaller {

    private static String CATALINASHRASP = "if \"%ACTION%\" == \"start\""
            + " set JAVA_OPTS=-javaagent:%CATALINA_HOME%\\rasp\\rasp.jar %JAVA_OPTS%";
    private static String CATALINASHLOG4J = "if \"%ACTION%\" == \"start\" set JAVA_OPTS=-Dlog4j.rasp.configuration="
            + "file:%CATALINA_HOME%\\rasp\\conf\\rasp-log4j.xml %JAVA_OPTS%";

    TomcatInstaller(String serverName, String serverRoot) {
        super(serverName, serverRoot);
    }

    @Override
    protected String getInstallPath(String serverRoot) {
        return serverRoot + "\\rasp";
    }

    @Override
    protected String getScript(String installPath) {
        return installPath + "\\..\\bin\\catalina.bat";
    }

    @Override
    protected String modifyStartScript(String content) throws RaspError {
        int modifyConfigState = NOTFOUND;
        StringBuilder sb = new StringBuilder();
        Scanner scanner = new Scanner(content);
        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            if (FOUND == modifyConfigState) {
                sb.append(CATALINASHRASP + LINE_SEP);
                sb.append(CATALINASHLOG4J + LINE_SEP);
                modifyConfigState = DONE;
            }
            if (DONE == modifyConfigState) {
                if (line.contains(CATALINASHRASP) || line.contains(CATALINASHLOG4J)) {
                    continue;
                }
            }
            if (line.startsWith(":setArgs") && NOTFOUND == modifyConfigState) {
                modifyConfigState = FOUND;
            }
            sb.append(line).append(LINE_SEP);
        }
        if (NOTFOUND == modifyConfigState) {
            throw new RaspError(E10001 + "\":setArgs\"");
        }
        return sb.toString();
    }

}
