/*
 * Copyright 2017-2020 Baidu Inc.
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

package com.baidu.rasp.install;

import com.baidu.rasp.App;
import com.baidu.rasp.Attacher;
import com.baidu.rasp.RaspError;
import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.yaml.snakeyaml.DumperOptions;
import org.yaml.snakeyaml.Yaml;

import java.io.*;
import java.net.URL;
import java.net.URLDecoder;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

import static com.baidu.rasp.RaspError.E10003;

/**
 * Created by OpenRASP on 5/10/17.
 * All rights reserved
 */
public abstract class BaseStandardInstaller implements Installer {
    private String serverName;
    protected String serverRoot;
    protected boolean needModifyScript = true;
    public static String resinPath;
    public static int NOTFOUND = 0;
    public static int FOUND = 1;
    public static int DONE = 2;
    public static String LINE_SEP = System.getProperty("line.separator");

    public BaseStandardInstaller(String serverName, String serverRoot) {
        this.serverName = serverName;
        this.serverRoot = serverRoot;
        resinPath = serverRoot;
    }

    @Override
    public void install() throws RaspError, IOException {
        boolean firstInstall = false;
        String jarPath = getLocalJarPath();
        File srcDir = new File(new File(jarPath).getParent() + File.separator + "rasp");
        if (!(srcDir.exists() && srcDir.isDirectory())) {
            srcDir.mkdirs();
        }
        File installDir = new File(getInstallPath(serverRoot));
        if (!installDir.exists()) {
            installDir.mkdir();
        }

        File configFile = new File(installDir.getCanonicalPath() + File.separator + "conf" + File.separator + "openrasp.yml");
        if (!configFile.exists()) {
            firstInstall = true;
        }
        if (!srcDir.getCanonicalPath().equals(installDir.getCanonicalPath())) {
            // 拷贝rasp文件夹
            System.out.println("Duplicating \"rasp\" directory\n- " + installDir.getCanonicalPath());
            File[] files = srcDir.listFiles();
            String uuid = UUID.randomUUID().toString().replace("-", "");
            if (files != null) {
                for (File file : files) {
                    File destFile = new File(installDir.getPath() + File.separator + file.getName());
                    if (file.isDirectory()) {
                        FileUtils.copyDirectory(file, destFile);
                    } else {
                        if (file.getName().endsWith(".jar") && destFile.exists()) {
                            File tmpJarDirectory = new File(installDir.getPath() + File.separator + "jar_tmp");
                            if (tmpJarDirectory.exists() && !tmpJarDirectory.isDirectory()) {
                                tmpJarDirectory.delete();
                            }
                            if (!tmpJarDirectory.exists()) {
                                tmpJarDirectory.mkdir();
                            }
                            String renameFileName = destFile.getName().
                                    substring(0, destFile.getName().length() - 4) + "-" + uuid + ".jar";
                            destFile.renameTo(new File(tmpJarDirectory.getPath() + File.separator + renameFileName));
                        }
                        FileUtils.copyFile(file, destFile);
                    }
                }
            }
            FileUtils.copyDirectory(srcDir, installDir);
        }

        System.out.println("Make \"rasp\" directory writable\n");
        modifyFolerPermission(installDir.getCanonicalPath());

        //安装rasp开启云控，删除官方插件
        if (App.url != null && App.appId != null && App.appSecret != null) {
            File plugin = new File(installDir.getCanonicalPath() + File.separator + "plugins" + File.separator + "official.js");
            if (plugin.exists()) {
                plugin.delete();
            }
        }
        // 生成配置文件
        if (!generateConfig(installDir.getPath(), firstInstall)) {
            System.exit(1);
        }

        if (needModifyScript) {
            generateStartScript(installDir.getPath());
            if (App.isAttach) {
                System.out.println("\nAttach the rasp to process: " + App.pid);
                new Attacher(App.pid + "", App.baseDir).doAttach(Attacher.MODE_INSTALL);
            }

            System.out.println("\nInstallation completed without errors.");
            if (!App.isAttach) {
                System.out.println("Please restart application server to take effect.");
            }
        }
    }

    private void generateStartScript(String installPath) throws RaspError, IOException {
        // 找到要修改的启动脚本
        File script = new File(getScript(installPath));
        if (!script.exists()) {
            throw new RaspError(E10003 + script.getAbsolutePath());
        }

        // 修改服务器启动脚本
        System.out.println("Updating startup script\n- " + script.getCanonicalPath());
        String original = read(script);
        String modified = modifyStartScript(original);
        write(script, modified);
    }

    private boolean generateConfig(String dir, boolean firstInstall) {
        try {
            String sep = File.separator;
            File target = new File(dir + sep + "conf" + sep + "openrasp.yml");

            System.out.println("Generating \"openrasp.yml\"\n- " + target.getAbsolutePath());
            if (target.exists() && App.keepConfig) {
                System.out.println("- Already exists and reserved openrasp.yml, continuing ..");
                return true;
            }
            if (target.exists() && !firstInstall) {
                File reserve = new File(dir + sep + "conf" + sep + "openrasp.yml.bak");
                if (!reserve.exists()) {
                    reserve.createNewFile();
                }
                FileOutputStream outputStream = new FileOutputStream(reserve);
                FileInputStream inputStream = new FileInputStream(target);
                IOUtils.copy(inputStream, outputStream);
                outputStream.close();
                inputStream.close();
                System.out.println("- Backed up openrasp.yml to openrasp.yml.bak");
            } else {
                System.out.println("- Create " + target.getAbsolutePath());
                target.getParentFile().mkdir();
                target.createNewFile();
            }
            FileOutputStream outputStream = new FileOutputStream(target);
            InputStream is = this.getClass().getResourceAsStream("/openrasp.yml");
            IOUtils.copy(is, outputStream);
            is.close();
            outputStream.close();

            // 配置云控
            setCloudConf();
            // 配置其它选项
            if (App.raspId != null) {
                setRaspConfItem("rasp.id", App.raspId, "# <rasp id>");
            }
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }
        return true;

    }

    private void setRaspConfItem(String key, String value, String comment) {
        Map<String, Object> ymlData = new HashMap<String, Object>();
        ymlData.put(key, value);
        setRaspConf(ymlData, comment);
    }

    public static String read(File file) throws IOException {
        FileInputStream fis = new FileInputStream(file);
        byte[] data = new byte[(int) file.length()];
        fis.read(data);
        fis.close();
        return new String(data);
    }

    public static void write(File file, String content) throws IOException {
        FileOutputStream outputStream = new FileOutputStream(file);
        OutputStreamWriter writer = new OutputStreamWriter(outputStream);
        writer.write(content);
        writer.close();
    }

    public static String runCommand(String[] args) {
        try {
            Process p = Runtime.getRuntime().exec(args);
            BufferedReader buffer = new BufferedReader(new InputStreamReader(p.getInputStream()));
            StringBuilder sb = new StringBuilder();
            String line;
            while ((line = buffer.readLine()) != null) {
                sb.append(line).append("\n");
            }
            if (sb.length() > 0) {
                sb.deleteCharAt(sb.length() - 1);
            }
            return sb.toString();
        } catch (IOException e) {
            return "";
        }
    }

    public static void modifyFolerPermission(String folderPath) {
        if (System.getProperty("os.name").startsWith("Windows")) {
            String res = runCommand(new String[]{"cmd", "/c", "echo Y|%SYSTEMROOT%\\System32\\cacls \"" + folderPath + "\" /G everyone:F"});
            // System.out.println(res);
        } else {
            if (System.getProperty("user.name").equals("root")) {
                runCommand(new String[]{"chmod", "-R", "go+w", folderPath});
            }
        }
    }

    private void setCloudConf() {
        if (App.url != null) {
            Map<String, Object> cloudData = new HashMap<String, Object>();
            cloudData.put("cloud.enable", true);
            cloudData.put("cloud.backend_url", App.url);
            cloudData.put("cloud.app_id", App.appId);
            cloudData.put("cloud.app_secret", App.appSecret);
            if (App.heartbeatInterval != null) {
                cloudData.put("cloud.heartbeat_interval", App.heartbeatInterval);
            }
            setRaspConf(cloudData, "# <remote management>");
        }
    }

    private void setRaspConf(Map<String, Object> ymlData, String comment) {
        try {
            String path = getInstallPath(serverRoot) + File.separator + "conf" + File.separator + "openrasp.yml";
            File yamlFile = new File(path);
            if (yamlFile.exists()) {
                BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(yamlFile, true), "UTF-8"));
                writer.write(LINE_SEP);
                writer.write(comment);
                writer.write(LINE_SEP);
                DumperOptions options = new DumperOptions();
                options.setDefaultFlowStyle(DumperOptions.FlowStyle.BLOCK);
                options.setPrettyFlow(true);
                Yaml yaml = new Yaml(options);
                yaml.dump(ymlData, writer);
            }
        } catch (Exception e) {
            System.out.println("Unable to update openrasp.yml: failed to set openrasp configuration: " + e.getMessage());
        }
    }

    //判断tomcat的版本是否大于8
    protected boolean checkTomcatVersion() {
        String javaVersion = System.getProperty("java.version");
        String[] version = javaVersion.split("\\.");
        if (version.length >= 2) {
            int major;
            int minor;
            try {
                major = Integer.parseInt(version[0]);
                minor = Integer.parseInt(version[1]);
            } catch (NumberFormatException e) {
                return false;
            }
            if (major == 1) {
                return minor >= 9;
            } else if (major >= 9) {
                return true;
            }
        } else if (javaVersion.startsWith("9")) {
            return true;
        } else if (javaVersion.length() >= 2) {
            char first = javaVersion.charAt(0);
            char second = javaVersion.charAt(1);
            if (first >= '1' && first <= '9' && second >= '0' && second <= '9') {
                return true;
            }
        }
        return false;
    }

    //获取指定目录下指定前缀的jar文件
    protected static String findFile(String path, final String prefix) {
        File baseDir = new File(path);
        if (baseDir.exists() && baseDir.isDirectory()) {
            File[] files = baseDir.listFiles(new FileFilter() {
                @Override
                public boolean accept(File file) {
                    return file.isFile() && file.getName().startsWith(prefix) &&
                            file.getName().endsWith(".jar");
                }
            });
            if (files != null && files.length == 1) {
                return files[0].getAbsolutePath();
            }
        }
        return null;
    }

    //获取当前所在jar包的路径
    public String getLocalJarPath() throws UnsupportedEncodingException {
        URL localUrl = getClass().getProtectionDomain().getCodeSource().getLocation();
        return URLDecoder.decode(localUrl.getFile().replace("+", "%2B"), "UTF-8");
    }

    protected abstract String getInstallPath(String serverRoot);

    protected abstract String getScript(String installPath);

    protected abstract String modifyStartScript(String content) throws RaspError;
}
