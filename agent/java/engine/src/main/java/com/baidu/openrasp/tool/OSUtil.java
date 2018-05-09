/*
 * Copyright 2017-2018 Baidu Inc.
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

import com.baidu.openrasp.tool.model.NicModel;

import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.util.Enumeration;
import java.util.LinkedList;

public class OSUtil {

    private static InetAddress inetAddress;

    public static String getHostName() {
        return inetAddress == null ? null : inetAddress.getHostName();
    }

    public static LinkedList<NicModel> getIpAddress() {
        LinkedList<NicModel> ipList = new LinkedList<NicModel>();
        try {
            if (inetAddress == null) {
                inetAddress = InetAddress.getLocalHost();
            }
            Enumeration allNetInterfaces = null;
            allNetInterfaces = NetworkInterface.getNetworkInterfaces();

            if (allNetInterfaces != null) {
                InetAddress ipAddress = null;
                while (allNetInterfaces.hasMoreElements()) {
                    NetworkInterface netInterface = (NetworkInterface) allNetInterfaces.nextElement();
                    Enumeration addresses = netInterface.getInetAddresses();
                    while (addresses.hasMoreElements()) {
                        ipAddress = (InetAddress) addresses.nextElement();
                        if (ipAddress != null && ipAddress instanceof Inet4Address) {
                            String ip = ipAddress.getHostAddress();
                            if (!ip.equals("0.0.0.0") && !ip.equals("127.0.0.1")) {
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

    public static boolean isWindows() {
        return System.getProperty("os.name") != null && System.getProperty("os.name").toLowerCase().contains("windows");
    }

    public static boolean isLinux() {
        return System.getProperty("os.name") != null && System.getProperty("os.name").toLowerCase().contains("linux");
    }

    public static boolean isMacOS(){
        return System.getProperty("os.name") != null && System.getProperty("os.name").toLowerCase().contains("mac os x");
    }

}
