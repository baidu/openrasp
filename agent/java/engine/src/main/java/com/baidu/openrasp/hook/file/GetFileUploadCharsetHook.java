package com.baidu.openrasp.hook.file;

import com.baidu.openrasp.hook.AbstractClassHook;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * Created by anyang on 2018/7/5.
 */
public class GetFileUploadCharsetHook extends AbstractClassHook {

    public static String charset;
    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/commons/fileupload/disk/DiskFileItem".equals(className);
    }

    @Override
    public String getType() {
        return null;
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {

        String src = getInvokeStaticSrc(GetFileUploadCharsetHook.class, "getCharset", "$_,DEFAULT_CHARSET", String.class,String.class);
        insertAfter(ctClass, "getCharSet", null, src);
    }

    public static void getCharset(String result,String defaultCharset){
        if (result!=null){
            charset=result;
        }else {
            charset=defaultCharset;
        }

    }

}
