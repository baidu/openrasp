/*
 * Copyright 2017-2021 Baidu Inc.
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

package com.baidu.openrasp.tool;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.messaging.ErrorType;
import com.baidu.openrasp.messaging.LogTool;
import com.baidu.openrasp.tool.model.NicModel;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang3.StringUtils;

import java.io.InputStream;
import java.math.BigInteger;
import java.net.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Collections;
import java.util.Enumeration;
import java.util.LinkedList;

public class OSUtil {

    public static String getHostName() {
        InetAddress inetAddress;
        try {
            inetAddress = InetAddress.getLocalHost();
        } catch (UnknownHostException e) {
            inetAddress = null;
        }
        if (inetAddress != null) {
            return inetAddress.getHostName();
        } else {
            return execReadToString();
        }
    }

    public static LinkedList<NicModel> getIpAddress() {
        LinkedList<NicModel> ipList = new LinkedList<NicModel>();
        try {
            Enumeration allNetInterfaces = null;
            allNetInterfaces = NetworkInterface.getNetworkInterfaces();

            if (allNetInterfaces != null) {
                InetAddress ipAddress = null;
                while (allNetInterfaces.hasMoreElements()) {
                    NetworkInterface netInterface = (NetworkInterface) allNetInterfaces.nextElement();
                    Enumeration addresses = netInterface.getInetAddresses();
                    while (addresses.hasMoreElements()) {
                        ipAddress = (InetAddress) addresses.nextElement();
                        if (ipAddress != null && ipAddress instanceof Inet4Address && !ipAddress.isLoopbackAddress()) {
                            String ip = ipAddress.getHostAddress();
                            if (!ip.equals("0.0.0.0")) {
                                ipList.add(new NicModel(netInterface.getName(), ipAddress.getHostAddress()));
                            }
                        }
                    }
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return ipList;
    }

    public static String getOs() {
        String os = System.getProperty("os.name");
        if (os == null) {
            return "";
        }
        os = os.toLowerCase();
        if (os.contains("linux")) return "Linux";
        if (os.contains("windows")) return "Windows";
        if (os.contains("mac")) return "Mac";
        if (os.contains("sunos")) return "SunOS";
        if (os.contains("freebsd")) return "FreeBSD";
        return os;
    }

    public static String getRaspId() throws Exception {
        if (!StringUtils.isEmpty(Config.getConfig().getRaspId())) {
            return Config.getConfig().getRaspId();
        }
        LinkedList<String> macs = OSUtil.getMacAddress();
        String macString = "";
        for (String mac : macs) {
            macString += mac;
        }
        MessageDigest md5 = MessageDigest.getInstance("md5");
        md5.update((macString + Config.getConfig().getBaseDirectory() + getHostName()).getBytes());
        BigInteger bigInt = new BigInteger(1, md5.digest());
        return bigInt.toString(16);
    }

    private static LinkedList<String> getMacAddress() throws SocketException {
        LinkedList<String> macs = new LinkedList<String>();

        Enumeration<NetworkInterface> el = null;
        try {
            el = NetworkInterface.getNetworkInterfaces();
        } catch (SocketException e) {
            LogTool.error(ErrorType.RUNTIME_ERROR, "failed to get mac information", e);
            throw e;
        }
        while (el.hasMoreElements()) {
            NetworkInterface netInterface = el.nextElement();
            try {
                if (!netInterface.isLoopback()) {
                    byte[] mac = netInterface.getHardwareAddress();
                    if (mac == null || mac.length == 0)
                        continue;
                    String macString = "";
                    for (byte b : mac) {
                        macString += (hexByte(b) + "-");
                    }
                    macs.add(macString.substring(0, macString.length() - 1));
                }
            } catch (SocketException e) {
                String message = "failed to handle mac information, name: " +
                        netInterface.getName() + ", DisplayName:" + netInterface.getDisplayName();
                System.out.println(message);
                LogTool.error(ErrorType.RUNTIME_ERROR, message, e);
            }
        }
        if (macs.isEmpty()) {
            throw new IllegalStateException("the mac information is empty");
        }
        Collections.sort(macs);
        return macs;
    }

    private static String hexByte(byte b) {
        String s = "0" + Integer.toHexString(b);
        return s.substring(s.length() - 2);
    }

    public static boolean isWindows() {
        return System.getProperty("os.name") != null && System.getProperty("os.name").toLowerCase().contains("windows");
    }

    public static boolean isLinux() {
        return System.getProperty("os.name") != null && System.getProperty("os.name").toLowerCase().contains("linux");
    }

    public static boolean isMacOS() {
        return System.getProperty("os.name") != null && System.getProperty("os.name").toLowerCase().contains("mac os x");
    }

    public static String getMasterIp(String requestUrl) throws Exception {
        URL url = new URL(requestUrl);
        Socket socket = new Socket();
        socket.connect(new InetSocketAddress(url.getHost(), getPort(url)), 10000);
        String ip = socket.getLocalAddress().getHostAddress();
        return ip != null ? ip : "";
    }

    public static int getPort(URL url) {
        int port = url.getPort();
        if (port < 0) {
            if ("https".equals(url.getProtocol().toLowerCase())) {
                port = 443;
            } else {
                port = 80;
            }
        }
        return port;
    }

    private static String execReadToString() {
        try {
            HookHandler.enableCmdHook.set(false);
            InputStream in = Runtime.getRuntime().exec("hostname").getInputStream();
            return IOUtils.toString(in);
        } catch (Exception e) {
            return "im-not-resolvable";
        } finally {
            HookHandler.enableCmdHook.set(true);
        }
    }

    public static String getDigestMd5(String data) throws NoSuchAlgorithmException {
        MessageDigest md5 = MessageDigest.getInstance("md5");
        md5.update(data.getBytes());
        BigInteger bigInt = new BigInteger(1, md5.digest());
        return bigInt.toString(16);
    }


}
