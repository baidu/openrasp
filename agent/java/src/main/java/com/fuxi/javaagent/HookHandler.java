/*
 * Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

package com.fuxi.javaagent;

import com.fuxi.javaagent.exception.SecurityException;
import com.fuxi.javaagent.hook.XXEHook;
import com.fuxi.javaagent.plugin.checker.CheckParameter;
import com.fuxi.javaagent.plugin.checker.CheckerManager;
import com.fuxi.javaagent.request.AbstractRequest;
import com.fuxi.javaagent.request.HttpServletRequest;
import com.fuxi.javaagent.response.HttpServletResponse;
import org.apache.log4j.Logger;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Created by zhuming01 on 5/16/17.
 * All rights reserved
 * 在hook增加检测入口的hook点处理类
 */
@SuppressWarnings("unused")
public class HookHandler {
    public static final String OPEN_RASP_HEADER_KEY = "X-Protected-By";
    public static final String OPEN_RASP_HEADER_VALUE = "OpenRASP";
    public static final String REQUEST_ID_HEADER_KEY = "X-Request-ID";
    public static final Logger LOGGER = Logger.getLogger(HookHandler.class.getName());
    // 全局开关
    public static AtomicBoolean enableHook = new AtomicBoolean(false);
    // 当前线程开关
    private static ThreadLocal<Boolean> enableCurrThreadHook = new ThreadLocal<Boolean>() {
        @Override
        protected Boolean initialValue() {
            return false;
        }
    };

    private static ThreadLocal<Boolean> tmpEnableCurrThreadHook = new ThreadLocal<Boolean>() {
        @Override
        protected Boolean initialValue() {
            return false;
        }
    };

    public static ThreadLocal<AbstractRequest> requestCache = new ThreadLocal<AbstractRequest>() {
        @Override
        protected AbstractRequest initialValue() {
            return null;
        }
    };
    public static ThreadLocal<HttpServletResponse> responseCache = new ThreadLocal<HttpServletResponse>() {
        @Override
        protected HttpServletResponse initialValue() {
            return null;
        }
    };

    private static final Map<String, Object> EMPTY_MAP = new HashMap<String, Object>();

    /**
     * 用于关闭当前的线程的hook点
     */
    public static void disableCurrThreadHook() {
        enableCurrThreadHook.set(false);
    }

    /**
     * 用于开启当前线程的hook点
     */
    public static void enableCurrThreadHook() {
        enableCurrThreadHook.set(true);
    }

    public static boolean isEnableCurrThreadHook() {
        return enableCurrThreadHook.get();
    }

    /**
     * 用于测试新增hook点，并把hook信息当做log打印
     *
     * @param className hook点类名
     * @param method    hook点方法名
     * @param desc      hook点方法描述符
     * @param args      hook点参数列表
     */
    public static void checkCommon(String className, String method, String desc, Object... args) {
        LOGGER.debug("checkCommon: " + className + ":" + method + ":" + desc + " " + Arrays.toString(args));
    }

    /**
     * 进入需要屏蔽hook的方法关闭开关
     */
    public static void preShieldHook() {
        tmpEnableCurrThreadHook.set(enableCurrThreadHook.get());
        disableCurrThreadHook();
    }

    /**
     * 退出需要屏蔽hook的方法打开开关
     */
    public static void postShieldHook() {
        if (tmpEnableCurrThreadHook.get()) {
            enableCurrThreadHook();
        }
    }

    /**
     * 请求进入hook点
     *
     * @param servlet  servlet对象
     * @param request  请求实体
     * @param response 响应实体
     */
    public static void checkRequest(Object servlet, Object request, Object response) {
        if (servlet != null && request != null && !enableCurrThreadHook.get()) {
            // 默认是关闭hook的，只有处理过HTTP request的线程才打开
            enableCurrThreadHook.set(true);
            HttpServletRequest requestContainer = new HttpServletRequest(request);
            HttpServletResponse responseContainer = new HttpServletResponse(response);
            responseContainer.setHeader(OPEN_RASP_HEADER_KEY, OPEN_RASP_HEADER_VALUE);
            responseContainer.setHeader(REQUEST_ID_HEADER_KEY, requestContainer.getRequestId());
            requestCache.set(requestContainer);
            responseCache.set(responseContainer);

            XXEHook.resetLocalExpandedSystemIds();

        }
    }

    /**
     * 请求结束hook点
     * 请求结束后不可以在进入任何hook点
     */
    public static void onServiceExit() {
        enableCurrThreadHook.set(false);
        requestCache.set(null);
    }

    /**
     * 在过滤器中进入的hook点
     *
     * @param filter   过滤器
     * @param request  请求实体
     * @param response 响应实体
     */
    public static void checkFilterRequest(Object filter, Object request, Object response) {
        checkRequest(filter, request, response);
    }

    public static void onInputStreamRead(int ret, Object inputStream) {
        if (ret != -1 && requestCache.get() != null) {
            AbstractRequest request = requestCache.get();
            if (request.getInputStream() == null) {
                request.setInputStream(inputStream);
            }
            if (request.getInputStream() == inputStream) {
                request.appendBody(ret);
            }
        }
    }

    public static void onInputStreamRead(int ret, Object inputStream, byte[] bytes) {
        if (ret != -1 && requestCache.get() != null) {
            AbstractRequest request = requestCache.get();
            if (request.getInputStream() == null) {
                request.setInputStream(inputStream);
            }
            if (request.getInputStream() == inputStream) {
                request.appendBody(bytes, 0, ret);
            }
        }
    }

    public static void onInputStreamRead(int ret, Object inputStream, byte[] bytes, int offset, int len) {
        if (ret != -1 && requestCache.get() != null) {
            AbstractRequest request = requestCache.get();
            if (request.getInputStream() == null) {
                request.setInputStream(inputStream);
            }
            if (request.getInputStream() == inputStream) {
                request.appendBody(bytes, offset, ret);
            }
        }
    }

    private static void handleBlock() {
        SecurityException securityException = new SecurityException("Request blocked by OpenRASP");
        if (responseCache.get() != null) {
            responseCache.get().sendError();
        }
        throw securityException;
    }

    /**
     * 无需在请求线程中执行的检测入口
     *
     * @param type   检测类型
     * @param params 检测参数map，key为参数名，value为检测参数值
     */
    public static void doCheckWithoutRequest(CheckParameter.Type type, Object params) {
        boolean enableHookCache = enableCurrThreadHook.get();
        try {
            enableCurrThreadHook.set(false);
            CheckParameter parameter = new CheckParameter(type, params);
            boolean isBlock = CheckerManager.check(type, parameter);
            if (isBlock) {
                handleBlock();
            }
        } catch (Exception e) {
            LOGGER.warn("plugin check error: " + e.getMessage());
        } finally {
            enableCurrThreadHook.set(enableHookCache);
        }
    }

    /**
     * 请求线程检测入口
     *
     * @param type   检测类型
     * @param params 检测参数map，key为参数名，value为检测参数值
     */
    public static void doCheck(CheckParameter.Type type, Object params) {
        if (enableHook.get() && enableCurrThreadHook.get()) {
            doCheckWithoutRequest(type, params);
        }
    }

}
