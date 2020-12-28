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

package com.baidu.openrasp.hook.server;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.request.AbstractRequest;

/**
 * Created by tyy on 18-2-11.
 *
 * 用于从服务器请求中获取 body 的 hook 点基类
 */
public abstract class ServerInputHook extends AbstractClassHook {

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "body";
    }

    public static void onCharRead(int character, Object reader) {
        if (character != -1 && HookHandler.requestCache.get() != null) {
            AbstractRequest request = HookHandler.requestCache.get();
            if (request.getCharReader() == null) {
                request.setCharReader(reader);
            }
            if (request.getCharReader() == reader) {
                request.appendCharBody(character);
            }
        }
    }

    public static void onCharRead(int ret, Object reader, char[] cbuf) {
        if (ret != -1 && HookHandler.requestCache.get() != null) {
            AbstractRequest request = HookHandler.requestCache.get();
            if (request.getCharReader() == null) {
                request.setCharReader(reader);
            }
            if (request.getCharReader() == reader) {
                request.appendBody(cbuf, 0, ret);
            }
        }
    }

    public static void onCharRead(int ret, Object reader, char[] cbuf, int off) {
        if (ret != -1 && HookHandler.requestCache.get() != null) {
            AbstractRequest request = HookHandler.requestCache.get();
            if (request.getCharReader() == null) {
                request.setCharReader(reader);
            }
            if (request.getCharReader() == reader) {
                request.appendBody(cbuf, off, ret);
            }
        }
    }

    public static void onCharReadLine(String line, Object reader) {
        if (line != null && HookHandler.requestCache.get() != null) {
            AbstractRequest request = HookHandler.requestCache.get();
            if (request.getCharReader() == null) {
                request.setCharReader(reader);
            }
            if (request.getCharReader() == reader) {
                request.appendBody((line + "\n").toCharArray(), 0, line.length() + 1);
            }
        }
    }

    public static void onInputStreamRead(int ret, Object inputStream) {
        if (ret != -1 && HookHandler.requestCache.get() != null) {
            AbstractRequest request = HookHandler.requestCache.get();
            if (request.getInputStream() == null) {
                request.setInputStream(inputStream);
            }
            if (request.getInputStream() == inputStream) {
                request.appendByteBody(ret);
            }
        }
    }

    public static void onInputStreamRead(int ret, Object inputStream, byte[] bytes) {
        if (ret != -1 && HookHandler.requestCache.get() != null) {
            AbstractRequest request = HookHandler.requestCache.get();
            if (request.getInputStream() == null) {
                request.setInputStream(inputStream);
            }
            if (request.getInputStream() == inputStream) {
                request.appendBody(bytes, 0, ret);
            }
        }
    }

    public static void onInputStreamRead(int ret, Object inputStream, byte[] bytes, int offset) {
        if (ret != -1 && HookHandler.requestCache.get() != null) {
            AbstractRequest request = HookHandler.requestCache.get();
            if (request.getInputStream() == null) {
                request.setInputStream(inputStream);
            }
            if (request.getInputStream() == inputStream) {
                request.appendBody(bytes, offset, ret);
            }
        }
    }

}
