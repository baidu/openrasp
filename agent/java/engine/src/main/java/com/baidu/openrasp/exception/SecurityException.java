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

package com.baidu.openrasp.exception;

/**
 * Created by tyy on 4/10/17.
 *
 * 用于做 hook 点拦截的异常
 */
public class SecurityException extends RuntimeException {

    /**
     * (none-javadoc)
     *
     * @see RuntimeException#RuntimeException()
     */
    public SecurityException() {
        super();
    }

    /**
     * constructor
     *
     * @param message 安全异常信息
     */
    public SecurityException(String message) {
        super(message);
    }

    @Override
    public StackTraceElement[] getStackTrace() {
        return new StackTraceElement[0];
    }
}
