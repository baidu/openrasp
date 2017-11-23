package com.baidu.rasp;

/**
 * Created by OpenRASP on 7/28/17.
 * All rights reserved
 */
public class RaspError extends Exception {

    public static String E10001 = "[error 10001] Insert mark not found in start script: ";
    public static String E10002 = "[error 10002] Server root directory not found： ";
    public static String E10003 = "[error 10003] Start script not found： ";

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

