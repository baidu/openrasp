/**
 *
 */
package com.baidu.openrasp.hook.server.tongweb7;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.server.ServerResponseBodyHook;
import com.baidu.openrasp.messaging.LogTool;
import com.baidu.openrasp.response.HttpServletResponse;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import com.baidu.openrasp.tool.model.ApplicationModel;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;
import java.util.HashMap;

/**
 * @description: Tongweb body_xss hook点
 * @author: zxl
 * @create: 2022-01-01
 */
@HookAnnotation
public class Tongweb7ResponseBodyHook extends ServerResponseBodyHook {

    @Override
    public boolean isClassMatched(String className) {
        return "com/tongweb/catalina/connector/OutputBuffer".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src1 = getInvokeStaticSrc(Tongweb7ResponseBodyHook.class, "getBufferFromByteArray", "$1,$2,$3", byte[].class, int.class, int.class);
        insertBefore(ctClass, "realWriteBytes", "([BII)V", src1);
    }

    public static void getBufferFromByteArray(byte[] buf, int off, int cnt) {
        boolean isCheckXss = isCheckXss();
        boolean isCheckSensitive = isCheckSensitive();
        if (HookHandler.isEnableXssHook() && (isCheckXss || isCheckSensitive)) {
            HookHandler.disableBodyXssHook();
            HashMap<String, Object> params = new HashMap<String, Object>();
            if (buf != null && cnt > 0) {
                try {
                    byte[] temp = new byte[cnt + 1];
                    System.arraycopy(buf, off, temp, 0, cnt);
                    String content = new String(temp);
                    params.put("content", content);
                    HttpServletResponse res = HookHandler.responseCache.get();
                    if (res != null) {
                        params.put("content_type", res.getContentType());
                    }
                } catch (Exception e) {
                    LogTool.traceHookWarn(ApplicationModel.getServerName() + " xss detection failed: " +
                            e.getMessage(), e);
                }
                if (HookHandler.requestCache.get() != null && !params.isEmpty()) {
                    checkBody(params, isCheckXss, isCheckSensitive);
                }
            }
        }
    }

}
