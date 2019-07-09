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

package com.baidu.openrasp;

import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.model.HookWhiteModel;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.exceptions.SecurityException;
import com.baidu.openrasp.hook.xxe.XXEHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.checker.CheckerManager;
import com.baidu.openrasp.request.AbstractRequest;
import com.baidu.openrasp.request.DubboRequest;
import com.baidu.openrasp.request.HttpServletRequest;
import com.baidu.openrasp.response.HttpServletResponse;
import org.apache.log4j.Logger;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicLong;

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
    public static AtomicLong requestSum = new AtomicLong(0);
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

    public static ThreadLocal<Boolean> enableXssHook = new ThreadLocal<Boolean>() {
        @Override
        protected Boolean initialValue() {
            return true;
        }
    };

    public static ThreadLocal<Boolean> enableCmdHook = new ThreadLocal<Boolean>() {
        @Override
        protected Boolean initialValue() {
            return true;
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
     * 用于关闭xss的hook点
     */
    public static void disableBodyXssHook() {
        enableXssHook.set(false);
    }

    /**
     * 用于开启xss的hook点
     */
    public static void enableBodyXssHook() {
        enableXssHook.set(true);
    }

    /**
     * 用于判断xss的hook点状态
     */
    public static boolean isEnableXssHook() {
        return enableXssHook.get();
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
            //新的请求开启body xss hook点
            enableBodyXssHook();
            HttpServletRequest requestContainer = new HttpServletRequest(request);
            HttpServletResponse responseContainer = new HttpServletResponse(response);
            responseContainer.setHeader(OPEN_RASP_HEADER_KEY, OPEN_RASP_HEADER_VALUE);
            responseContainer.setHeader(REQUEST_ID_HEADER_KEY, requestContainer.getRequestId());
            //设置响应的用户自定义头部
            setUserDefinedResponseHeader(responseContainer);
            requestCache.set(requestContainer);
            responseCache.set(responseContainer);
            XXEHook.resetLocalExpandedSystemIds();
            doCheck(CheckParameter.Type.REQUEST, new Object());
        }
    }

    /**
     * 请求进入Dubbo的hook点
     *
     * @param request 请求实体
     */
    public static void checkDubboRequest(Object request) {
        if (request != null && !enableCurrThreadHook.get()) {
            enableCurrThreadHook.set(true);
            //新的请求开启body xss hook点
            enableBodyXssHook();
            DubboRequest requestContainer = new DubboRequest(request);
            requestCache.set(requestContainer);
            XXEHook.resetLocalExpandedSystemIds();
            doCheck(CheckParameter.Type.REQUEST, new Object());
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
     * 请求结束dubbo hook点
     * 请求结束后不可以在进入任何hook点
     */
    public static void onDubboExit() {
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
        requestSum.incrementAndGet();
        checkRequest(filter, request, response);
    }

    /**
     * 在过滤器中进入dubbo的hook点
     *
     * @param request 请求实体
     */
    public static void checkDubboFilterRequest(Object request) {
        checkDubboRequest(request);
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

    public static void onParseParameters() {
        AbstractRequest request = requestCache.get();
        if (request != null) {
            request.setCanGetParameter(true);
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
     * 设置用户的自定义header
     *
     * @param response http请求的response
     */
    private static void setUserDefinedResponseHeader(HttpServletResponse response) {
        Map<String, String> headers = Config.getConfig().getResponseHeaders();
        if (headers != null && !headers.isEmpty()) {
            for (Map.Entry<String, String> entry : headers.entrySet()) {
                response.setHeader(entry.getKey(), entry.getValue());
            }
        }
    }

    /**
     * 检测入口
     *
     * @param type   检测类型
     * @param params 检测参数map，key为参数名，value为检测参数值
     */
    public static void doRealCheckWithoutRequest(CheckParameter.Type type, Object params) {
        if (!enableHook.get()) {
            return;
        }
        long a = 0;
        if (Config.getConfig().getDebugLevel() > 0) {
            a = System.currentTimeMillis();
        }
        boolean enableHookCache = enableCurrThreadHook.get();
        boolean isBlock = false;
        try {
            enableCurrThreadHook.set(false);
            CheckParameter parameter = new CheckParameter(type, params);
            isBlock = CheckerManager.check(type, parameter);
        } catch (Exception e) {
            String message = "plugin check error: " + e.getClass().getName() + " because: " + e.getMessage();
            int errorCode = ErrorType.PLUGIN_ERROR.getCode();
            LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
        } finally {
            enableCurrThreadHook.set(enableHookCache);
        }
        if (a > 0) {
            long t = System.currentTimeMillis() - a;
            if (requestCache.get() != null) {
                LOGGER.info("request_id=" + requestCache.get().getRequestId() + " " + "type=" + type.getName() + " " + "time=" + t);
            }
        }
        if (isBlock) {
            handleBlock();
        }
    }

    /**
     * 无需在请求线程中执行的检测入口
     *
     * @param type   检测类型
     * @param params 检测参数map，key为参数名，value为检测参数值
     */
    public static void doCheckWithoutRequest(CheckParameter.Type type, Object params) {
        if (Config.getConfig().getCloudSwitch() && Config.getConfig().getHookWhiteAll()) {
            return;
        }
        if (requestCache.get() != null) {
            StringBuffer sb = requestCache.get().getRequestURL();
            if (sb != null) {
                String url = sb.substring(sb.indexOf("://") + 3);
                if (HookWhiteModel.isContainURL(type.getCode(), url)) {
                    return;
                }
            }
        }
        doRealCheckWithoutRequest(type, params);
    }

    /**
     * 请求线程检测入口
     *
     * @param type   检测类型
     * @param params 检测参数map，key为参数名，value为检测参数值
     */
    public static void doCheck(CheckParameter.Type type, Object params) {
        if (enableCurrThreadHook.get()) {
            doCheckWithoutRequest(type, params);
        }
    }

}
