/**
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

import com.fuxi.javaagent.config.Config;
import com.fuxi.javaagent.exception.SecurityException;
import com.fuxi.javaagent.hook.SQLStatementHook;
import com.fuxi.javaagent.hook.XXEHook;
import com.fuxi.javaagent.plugin.CheckParameter;
import com.fuxi.javaagent.plugin.PluginManager;
import com.fuxi.javaagent.request.AbstractRequest;
import com.fuxi.javaagent.request.HttpServletRequest;
import com.fuxi.javaagent.response.HttpServletResponse;
import com.fuxi.javaagent.tool.Reflection;
import com.fuxi.javaagent.tool.StackTrace;
import com.fuxi.javaagent.tool.hook.CustomLockObject;
import com.fuxi.javaagent.tool.security.tomcat.TomcatSecurityChecker;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.IOException;
import java.io.ObjectStreamClass;
import java.io.UnsupportedEncodingException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
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
    private static final Logger LOGGER = Logger.getLogger(HookHandler.class.getName());
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
    private static ThreadLocal<HttpServletResponse> responseCache = new ThreadLocal<HttpServletResponse>() {
        @Override
        protected HttpServletResponse initialValue() {
            return null;
        }
    };

    private static final Map<String, Object> EMPTY_MAP = new HashMap<String, Object>();

    /**
     * 用于关闭当前的线程的hook点
     */
    private static void disableCurrThreadHook() {
        enableCurrThreadHook.set(false);
    }

    /**
     * 用于开启当前线程的hook点
     */
    private static void enableCurrThreadHook() {
        enableCurrThreadHook.set(true);
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
     * 文件上传hook点
     *
     * @param name    文件名
     * @param content 文件数据
     */
    public static void checkFileUpload(String name, byte[] content) {
        if (name != null && content != null) {
            Map<String, Object> params = new HashMap<String, Object>();
            params.put("filename", name);
            try {
                if (content.length > 4 * 1024) {
                    content = Arrays.copyOf(content, 4 * 1024);
                }
                params.put("content", new String(content, "UTF-8"));
            } catch (UnsupportedEncodingException e) {
                e.printStackTrace();
                params.put("content", "[rasp error:" + e.getMessage() + "]");
            }

            doCheck(CheckParameter.Type.FILEUPLOAD, params);
        }
    }

    /**
     * 列出文件列表方法hook点
     *
     * @param file 文件对象
     */
    public static void checkListFiles(File file) {
        if (file != null) {
            Map<String, Object> params = new HashMap<String, Object>();
            params.put("path", file.getPath());
            try {
                params.put("realpath", file.getCanonicalPath());
            } catch (IOException e) {
                params.put("realpath", file.getAbsolutePath());
            }

            doCheck(CheckParameter.Type.DIRECTORY, params);
        }
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
     * SQL语句检测
     *
     * @param stmt sql语句
     */
    public static void checkSQL(String server, Object statement, String stmt) {
        if (stmt != null && !stmt.isEmpty()) {
            Map<String, Object> params = new HashMap<String, Object>();
            String connectionId = SQLStatementHook.getSqlConnectionId(server, statement);
            if (connectionId != null) {
                params.put(server + "_connection_id", connectionId);
            }
            params.put("server", server);
            params.put("query", stmt);

            doCheck(CheckParameter.Type.SQL, params);
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
            doCheck(CheckParameter.Type.REQUEST, EMPTY_MAP);

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

    /**
     * ApplicationFilter中doFilter退出hook点
     */
    public static void onApplicationFilterExit() {
        onServiceExit();
    }

    /**
     * 文件读取hook点
     *
     * @param file 文件对象
     */
    public static void checkReadFile(File file) {
        if (file != null) {
            HashMap<String, Object> param = new HashMap<String, Object>();
            param.put("path", file.getPath());
            try {
                String path = file.getCanonicalPath();
                if (path.endsWith(".class")) {
                    return;
                }
                param.put("realpath", path);
            } catch (IOException e) {
                e.printStackTrace();
            }

            doCheck(CheckParameter.Type.READFILE, param);
        }
    }

    /**
     * 命令执行hook点
     *
     * @param command 命令列表
     */
    public static void checkCommand(List<String> command) {
        if (command != null && !command.isEmpty()) {
            HashMap<String, Object> param = new HashMap<String, Object>();
            param.put("command", command);
            doCheck(CheckParameter.Type.COMMAND, param);
        }
    }

    /**
     * xml语句解析hook点
     *
     * @param expandedSystemId
     */
    public static void checkXXE(String expandedSystemId) {
        if (expandedSystemId != null && !XXEHook.getLocalExpandedSystemIds().contains(expandedSystemId)) {
            XXEHook.getLocalExpandedSystemIds().add(expandedSystemId);
            HashMap<String, Object> param = new HashMap<String, Object>();
            param.put("entity", expandedSystemId);
            doCheck(CheckParameter.Type.XXE, param);
        }
    }

    /**
     * 写文件hook点
     *
     * @param file
     */
    public static void checkWriteFile(File file) {
        if (file != null) {
            HashMap<String, Object> param = new HashMap<String, Object>();
            param.put("name", file.getName());
            try {
                param.put("realpath", file.getCanonicalPath());
            } catch (IOException e) {
                e.printStackTrace();
            }
            param.put("content", "");
            doCheck(CheckParameter.Type.WRITEFILE, param);
        }
    }

    /**
     * 文件写入hook点
     *
     * @param closeLock  缓存文件信息
     * @param writeBytes 写的内容
     */
    public static void checkFileOutputStreamWrite(Object closeLock, byte[] writeBytes) {
        if (closeLock instanceof CustomLockObject && ((CustomLockObject) closeLock).getInfo() != null) {
            String path = ((CustomLockObject) closeLock).getInfo();
            if (path != null && writeBytes != null && writeBytes.length > 0) {
                File file = new File(path);
                Map<String, Object> params = new HashMap<String, Object>();
                params.put("name", file.getName());
                params.put("realpath", path);
                params.put("content", new String(writeBytes));
                doCheck(CheckParameter.Type.WRITEFILE, params);
            }
        }
    }

    /**
     * 文件输出流的构造函数hook点
     *
     * @param closeLock 用于记录文件信息的锁对象
     * @param path      文件路径
     */
    public static void checkFileOutputStreamInit(Object closeLock, String path) {
        if (closeLock instanceof CustomLockObject && enableHook.get() && enableCurrThreadHook.get()) {
            ((CustomLockObject) closeLock).setInfo(path);
        }
    }

    /**
     * struct框架ognl语句解析hook点
     *
     * @param expression ognl语句
     */
    public static void checkOgnlExpression(String expression) {
        if (expression != null) {
            Map<String, Object> params = new HashMap<String, Object>();
            params.put("expression", expression);
            doCheck(CheckParameter.Type.OGNL, params);
        }
    }

    /**
     * 反序列化监检测点
     *
     * @param objectStreamClass 反序列化的类的流对象
     */
    public static void checkDeserializationClass(ObjectStreamClass objectStreamClass) {
        if (objectStreamClass != null) {
            String clazz = objectStreamClass.getName();
            if (clazz != null) {
                Map<String, Object> params = new HashMap<String, Object>();
                params.put("clazz", clazz);
                doCheck(CheckParameter.Type.DESERIALIZATION, params);
            }
        }
    }


    /**
     * tomcat启动时检测安全规范
     */
    public static void checkTomcatStartup() {
        TomcatSecurityChecker checker = new TomcatSecurityChecker();
        boolean isSafe = checker.check();
        if (!isSafe) {
            if (Config.getConfig().getEnforcePolicy()) {
                throw new SecurityException("Can not startup tomcat:\n" + checker.getFormattedUnsafeMessage());
            }
        }
    }

    /**
     * 反射hook点检测
     *
     * @param method 反射调用的方法
     */
    public static void checkReflection(Object method) {
        if (enableHook.get() && enableCurrThreadHook.get()) {
            enableCurrThreadHook.set(false);
            try {
                Class reflectClass = (Class) Reflection.invokeMethod(method, "getDeclaringClass", new Class[]{});
                String reflectClassName = reflectClass.getName();
                String reflectMethodName = (String) Reflection.invokeMethod(method, "getName", new Class[]{});
                String absoluteMethodName = reflectClassName + "." + reflectMethodName;
                String[] reflectMonitorMethod = Config.getConfig().getReflectionMonitorMethod();
                for (String monitorMethod : reflectMonitorMethod) {
                    if (monitorMethod.equals(absoluteMethodName)) {
                        Map<String, Object> params = new HashMap<String, Object>();
                        List<String> stackInfo = StackTrace.getStackTraceArray(Config.REFLECTION_STACK_START_INDEX,
                                Config.getConfig().getReflectionMaxStack());
                        params.put("clazz", reflectClassName);
                        params.put("method", reflectMethodName);
                        params.put("stack", stackInfo);
                        pluginCheck(CheckParameter.Type.REFLECTION, params);
                        break;
                    }
                }
            } finally {
                enableCurrThreadHook.set(true);
            }
        }
    }

    /**
     * 检测WebDAV COPY MOVE
     *
     * @param webdavServlet
     * @param source
     * @param dest
     */
    public static void checkWebdavCopyResource(Object webdavServlet, String source, String dest) {
        if (webdavServlet != null && source != null && dest != null) {
            String realPath = null;
            try {
                Object servletContext = Reflection.invokeMethod(webdavServlet, "getServletContext", new Class[]{});
                realPath = Reflection.invokeStringMethod(servletContext, "getRealPath", new Class[]{String.class}, "/");
                realPath = realPath.endsWith(System.getProperty("file.separator")) ? realPath.substring(0, realPath.length() - 1) : realPath;
            } catch (Exception e) {
                e.printStackTrace();
            }
            if (realPath != null) {
                HashMap<String, Object> param = new HashMap<String, Object>();
                param.put("source", realPath + source);
                param.put("dest", realPath + dest);
                doCheck(CheckParameter.Type.WEBDAV, param);
            }
        }
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

    private static void handleBlock(CheckParameter parameter) {
        SecurityException securityException = new SecurityException("Request blocked by OpenRASP");
        if (responseCache.get() != null) {
            responseCache.get().sendError();
        }
        throw securityException;
    }

    /**
     * 检测插件入口
     *
     * @param type   检测类型
     * @param params 检测参数map，key为参数名，value为检测参数值
     */
    private static void doCheck(CheckParameter.Type type, Map<String, Object> params) {
        if (enableHook.get() && enableCurrThreadHook.get()) {
            enableCurrThreadHook.set(false);
            pluginCheck(type, params);
        }
    }

    private static void pluginCheck(CheckParameter.Type type, Map<String, Object> params) {
        CheckParameter parameter = new CheckParameter(type, params, requestCache.get());
        boolean isBlock = false;
        try {
            isBlock = PluginManager.check(parameter);
        } catch (Exception e) {
            LOGGER.warn("plugin check error: " + parameter);
        } finally {
            enableCurrThreadHook.set(true);
        }
        if (isBlock) {
            handleBlock(parameter);
        }
    }
}
