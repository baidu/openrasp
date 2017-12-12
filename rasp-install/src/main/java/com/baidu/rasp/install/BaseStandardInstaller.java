package com.baidu.rasp.install;

import com.baidu.rasp.RaspError;
import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;

import java.io.*;

import static com.baidu.rasp.RaspError.E10003;

/**
 * Created by OpenRASP on 5/10/17.
 * All rights reserved
 */
public abstract class BaseStandardInstaller implements Installer {
    private String serverName;
    private String serverRoot;
    public static int NOTFOUND = 0;
    public static int FOUND = 1;
    public static int DONE = 2;
    public static String LINE_SEP = System.getProperty("line.separator");

    public BaseStandardInstaller(String serverName, String serverRoot) {
        this.serverName = serverName;
        this.serverRoot = serverRoot;
    }

    @Override
    public void install() throws RaspError, IOException {
        String jarPath = getClass().getProtectionDomain().getCodeSource().getLocation().getPath();
        File srcDir = new File(new File(jarPath).getParent() + "/rasp");
        if (!(srcDir.exists() && srcDir.isDirectory())) {
            srcDir = new File("rasp");
        }
        File installDir = new File(getInstallPath(serverRoot));

        if (!srcDir.getCanonicalPath().equals(installDir.getCanonicalPath())) {
            // 拷贝rasp文件夹
            System.out.println("Duplicating \"rasp\" directory\n- " + installDir.getCanonicalPath());
            FileUtils.copyDirectory(srcDir, installDir);
        }

        // 生成配置文件
        if (!generateConfig(installDir.getPath())) {
            System.exit(1);
        }

        // 找到要修改的启动脚本
        File script = new File(getScript(installDir.getPath()));
        if (!script.exists()) {
            throw new RaspError(E10003 + script.getAbsolutePath());
        }

        // 修改服务器启动脚本
        System.out.println("Updating startup script\n- " + script.getCanonicalPath());
        String original = read(script);
        String modified = modifyStartScript(original);
        write(script, modified);
        System.out.println("\nInstallation completed without errors.\nPlease restart application server to take effect.");
    }


    private boolean generateConfig(String dir) {
        try {
            String sep = File.separator;
            File target = new File(dir + sep + "conf" + sep + "rasp.properties");


            System.out.println("Generating \"rasp.properties\"\n- " + target.getAbsolutePath());
            if (target.exists()) {
                System.out.println("- Already exists, continuing ..");
                return true;
            }
            System.out.println("create " + target.getAbsolutePath());
            target.getParentFile().mkdir();
            target.createNewFile();
            FileWriter writer = new FileWriter(target);
            InputStream is = this.getClass().getResourceAsStream("/rasp.properties");
            IOUtils.copy(is, writer, "UTF-8");
            is.close();
            writer.close();
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }
        return true;

    }

    protected String read(File file) throws IOException {
        FileInputStream fis = new FileInputStream(file);
        byte[] data = new byte[(int) file.length()];
        fis.read(data);
        fis.close();
        return new String(data);
    }

    protected void write(File file, String content) throws IOException {
        FileOutputStream outputStream = new FileOutputStream(file);
        OutputStreamWriter writer = new OutputStreamWriter(outputStream);
        writer.write(content);
        writer.close();
    }

    public String runCommand(String[] args) {
        try {
            Process p = Runtime.getRuntime().exec(args);
            BufferedReader buffer = new BufferedReader(new InputStreamReader(p.getInputStream()));
            StringBuilder sb = new StringBuilder();
            String line;
            while ((line = buffer.readLine()) != null) {
                sb.append(line).append("\n");
            }
            return sb.toString();
        } catch (IOException e) {
            return "";
        }
    }

    protected abstract String getInstallPath(String serverRoot);

    protected abstract String getScript(String installPath);

    protected abstract String modifyStartScript(String content) throws RaspError;
}
