package com.baidu.openrasp.hook.file;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.File;
import java.io.IOException;
import java.util.HashMap;

/**
 * springboot框架文件上传hook点
 * <p>
 * 参考FileUploadItemHook
 *
 * @author jinlong-ghq
 */
@HookAnnotation
public class FileUploadSpringframeworkHook extends AbstractClassHook {

    @Override
    public boolean isClassMatched(String className) {
        return "org/springframework/web/multipart/support/StandardMultipartHttpServletRequest$StandardMultipartFile".equals(className);
    }

    @Override
    public String getType() {
        return "fileUpload";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(FileUploadSpringframeworkHook.class,
                "checkFileItemWithBytes", "$1", File.class);
        insertBefore(ctClass, "transferTo", "(Ljava/io/File;)V", src);

    }

    /**
     * 检查文件项（带字节）
     *
     * @param dest 文件
     */
    public static void checkFileItemWithBytes(File dest) {
        HashMap<String, Object> params = new HashMap<String, Object>();
        params.put("filename", dest.getName());
        HookHandler.doCheck(CheckParameter.Type.FILEUPLOAD, params);
    }

}
