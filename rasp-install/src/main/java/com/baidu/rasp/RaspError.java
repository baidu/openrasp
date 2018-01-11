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

package com.baidu.rasp;

/**
 * Created by OpenRASP on 7/28/17.
 * All rights reserved
 */
public class RaspError extends Exception {

    public static String E10001 = "[ERROR 10001] Insert mark not found in start script: ";
    public static String E10002 = "[ERROR 10002] No such file or directory: ";
    public static String E10003 = "[ERROR 10003] Start script not found: ";
    public static String E10004 = "[ERROR 10004] Unable to determine application server type in: ";

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

