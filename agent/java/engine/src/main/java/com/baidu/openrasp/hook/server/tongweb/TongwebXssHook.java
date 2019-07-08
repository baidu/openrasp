/**
 *
 */
package com.baidu.openrasp.hook.server.tongweb;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.hook.server.ServerXssHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import com.baidu.openrasp.tool.model.ApplicationModel;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;
import java.util.HashMap;

/**
 * @description: Tongweb body_xss hook点
 * @author: Baimo
 * @create: 2019/06/19
 */
@HookAnnotation
public class TongwebXssHook extends ServerXssHook {

    @Override
    public boolean isClassMatched(String className) {
        return "com/tongweb/web/thor/connector/OutputBuffer".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src1 = getInvokeStaticSrc(TongwebXssHook.class, "getBufferFromByteArray", "$1,$2,$3", byte[].class, int.class, int.class);
        insertBefore(ctClass, "realWriteBytes", "([BII)V", src1);
    }

    public static void getBufferFromByteArray(byte[] buf, int off, int cnt) {
        if (HookHandler.isEnableXssHook()) {
            HookHandler.disableBodyXssHook();
            HashMap<String, Object> params = new HashMap<String, Object>();
            if (buf != null && cnt > 0) {
                try {
                    byte[] temp = new byte[cnt + 1];
                    System.arraycopy(buf, off, temp, 0, cnt);
                    String content = new String(temp);
                    params.put("html_body", content);

                } catch (Exception e) {
                    String message = ApplicationModel.getServerName() + " xss detectde failed";
                    int errorCode = ErrorType.HOOK_ERROR.getCode();
                    HookHandler.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
                }
                if (HookHandler.requestCache.get() != null && !params.isEmpty()) {
                    HookHandler.doCheck(CheckParameter.Type.XSS_USERINPUT, params);
                }
            }
        }
    }

}
