package com.baidu.openrasp.hook.file;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.messaging.LogTool;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.StackTrace;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.File;
import java.io.IOException;
import java.nio.file.Path;
import java.util.HashMap;
import java.util.List;

/**
 * nio files list hook
 * liergou
 * 2020.7.10
 */
public class NioFilesListHook extends AbstractClassHook {
    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "directory";
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "java/nio/file/Files".equals(className);
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(NioFilesListHook.class, "checkFileList", "$1", Path.class);
        insertBefore(ctClass, "newDirectoryStream", "(Ljava/nio/file/Path;Ljava/nio/file/DirectoryStream$Filter;)Ljava/nio/file/DirectoryStream;", src);
        insertBefore(ctClass, "walk", "(Ljava/nio/file/Path;I[Ljava/nio/file/FileVisitOption;)Ljava/util/stream/Stream;", src);
        insertBefore(ctClass, "walkFileTree", "(Ljava/nio/file/Path;Ljava/util/Set;ILjava/nio/file/FileVisitor;)Ljava/nio/file/Path;", src);
        insertBefore(ctClass, "find", "(Ljava/nio/file/Path;ILjava/util/function/BiPredicate;[Ljava/nio/file/FileVisitOption;)Ljava/util/stream/Stream;", src);
    }

    /**
     * nio file list hook
     *
     * @param path nio file path
     */
    public static void checkFileList(Path path) {
        boolean checkSwitch = Config.getConfig().getPluginFilter();
        File file=path.toFile();

        if (path.toString() != null) {
            if (checkSwitch && !file.exists()) {
                return;
            }
            HashMap<String, Object> params = null;
            try {
                params = new HashMap<String, Object>();
                params.put("path", file.getPath());
                List<String> stackInfo = StackTrace.getParamStackTraceArray();
                params.put("stack", stackInfo);
                try {
                    params.put("realpath", file.getCanonicalPath());
                } catch (Exception e) {
                    params.put("realpath", file.getAbsolutePath());
                }
            } catch (Throwable t) {
                LogTool.traceHookWarn(t.getMessage(), t);
            }
            HookHandler.doCheck(CheckParameter.Type.DIRECTORY, params);
        }
    }

}