/*
 * Copyright 2017-2019 Baidu Inc.
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

package com.baidu.openrasp.exceptions;

/**
 * Created by tyy on 19-1-24.
 * 处理 Attach 模式启动情况下的异常信息
 */
public class AttachRaspException extends RuntimeException {

    /**
     * constructor
     *
     * @param message Attach 启动异常信息
     */
    public AttachRaspException(String message) {
        super(message);
    }

}
