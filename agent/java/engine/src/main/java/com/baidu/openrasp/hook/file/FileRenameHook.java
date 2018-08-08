package com.baidu.openrasp.hook.file;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.js.engine.JSContext;
import com.baidu.openrasp.plugin.js.engine.JSContextFactory;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;
import org.mozilla.javascript.Scriptable;

import java.io.File;
import java.io.IOException;

/**
 * 　　* @Description: 文件改名hook点
 * 　　* @author anyang
 * 　　* @date 2018/7/23 10:54
 */
public class FileRenameHook extends AbstractClassHook {
    @Override
    public boolean isClassMatched(String className) {
        return "java/io/File".equals(className);
    }

    @Override
    public String getType() {
        return "rename";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {

        String src = getInvokeStaticSrc(FileRenameHook.class, "checkFileRename", "$0,$1", File.class, File.class);
        insertBefore(ctClass, "renameTo", "(Ljava/io/File;)Z", src);
    }

    public static void checkFileRename(File source, File dest) {
        if (source != null && dest != null) {

            JSContext cx = JSContextFactory.enterAndInitContext();
            Scriptable params = cx.newObject(cx.getScope());
            try {
                params.put("source", params, source.getCanonicalPath());
            } catch (IOException e) {
                params.put("source", params, source.getAbsolutePath());
            }

            try {
                params.put("dest", params, dest.getCanonicalPath());
            } catch (IOException e) {
                params.put("dest", params, dest.getAbsolutePath());
            }

            HookHandler.doCheck(CheckParameter.Type.FILERENAME, params);

        }
    }
}
