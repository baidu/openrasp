package com.baidu.rasp;

/**
 * Created by OpenRASP on 7/28/17.
 * All rights reserved
 */
public class RaspError extends Exception {

    public static String E10001 = "[ERROR 10001] Insert mark not found in start script: ";
    public static String E10002 = "[ERROR 10002] Server root directory not found: ";
    public static String E10003 = "[ERROR 10003] Start script not found: ";
    public static String E10004 = "[ERROR 10004] Unable to determine server type in: ";

    public RaspError(String message) {
        super(message);
    }

    public RaspError(String message, Throwable cause) {
        super(message, cause);
    }

    public RaspError(Throwable cause) {
        super(cause);
    }
}

