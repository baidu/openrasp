package com.baidu.openrasp.hook.server.inforsuite;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.server.ServerInputHook;
import com.baidu.openrasp.messaging.LogTool;
import com.baidu.openrasp.request.AbstractRequest;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import com.baidu.openrasp.tool.model.ApplicationModel;

import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * @description: inforsuite input buffer hook,将得到的buffer信息在本类中处理
 * @author: inforsuite
 * @create: 2022/05/20
 */
@HookAnnotation
public class InforSuiteInputBufferHook extends ServerInputHook {

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {

        if ("com/cvicse/inforsuite/grizzly/http/io/InputBuffer".equals(className)){
            return true;
        }
        return false;
    
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String bufferSrc = getInvokeStaticSrc(InforSuiteInputBufferHook.class, "onInputStreamRead",
                "$_", Object.class);
        insertAfter(ctClass, "getBuffer",null, bufferSrc);      
        String readSrc = getInvokeStaticSrc(ServerInputHook.class, "onInputStreamRead",
                "$_,$0,$1,$2", int.class, Object.class, byte[].class, int.class);
        insertAfter(ctClass, "read", "([BII)I", readSrc);
    }
    
    //handle inputStream
    public static void onInputStreamRead(Object inputStream) {
        if (HookHandler.requestCache.get() != null) {
            AbstractRequest request = HookHandler.requestCache.get();
            
            if (request.getInputStream() == null) {
                request.setInputStream(inputStream);
            }
            if (request.getInputStream() == inputStream) {
            	try {
            		byte[] heap = (byte[])Reflection.getSuperField(inputStream, "heap");
            		Integer offset = (Integer) Reflection.getSuperField(inputStream, "offset");
            		Integer cap = (Integer) Reflection.getSuperField(inputStream, "cap");           		
					request.appendBody(heap, offset, cap);
				} catch (Exception e) {
					LogTool.traceHookWarn(ApplicationModel.getServerName() + " get request body failed: " +
	                        e.getMessage(), e);
				} 
            }
        }
    }
   // end
}
