package com.baidu.openrasp.hook.file;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;
import java.nio.file.Path;
import java.util.HashMap;

/**
 * nio file move and copy and link hook
 * liergou
 * 2020.07.10
 */
public class NioFilesMoveHook extends AbstractClassHook {
    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "rename";
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
        String src = getInvokeStaticSrc(NioFilesMoveHook.class, "checkFileRename", "$1,$2", Path.class,Path.class);
        insertBefore(ctClass, "copy", "(Ljava/nio/file/Path;Ljava/nio/file/Path;[Ljava/nio/file/CopyOption;)Ljava/nio/file/Path;", src);
        insertBefore(ctClass, "move", "(Ljava/nio/file/Path;Ljava/nio/file/Path;[Ljava/nio/file/CopyOption;)Ljava/nio/file/Path;", src);
        String srcLink = getInvokeStaticSrc(NioFilesMoveHook.class, "checkFileLink", "$1,$2", Path.class,Path.class);
        insertBefore(ctClass, "createSymbolicLink", "(Ljava/nio/file/Path;Ljava/nio/file/Path;[Ljava/nio/file/attribute/FileAttribute;)Ljava/nio/file/Path;", srcLink);
        insertBefore(ctClass, "createLink", "(Ljava/nio/file/Path;Ljava/nio/file/Path;)Ljava/nio/file/Path;", srcLink);

    }

    /**
     * copy move hook；logic same as rename
     *
     * @param pathSource 源文件路径
     * @param pathDest 目标文件路径
     */
    public static void checkFileRename(Path pathSource,Path pathDest) {
        boolean checkSwitch = Config.getConfig().getPluginFilter();
        if (pathSource != null && !pathSource.toFile().isDirectory() && pathDest != null && !pathDest.toFile().isDirectory()) {
            if (checkSwitch && !pathSource.toFile().exists()){
                return;
            }
            HashMap<String, Object> params = new HashMap<String, Object>();
            try {
                params.put("source", pathSource.toFile().getCanonicalPath());
            } catch (IOException e) {
                params.put("source", pathSource.toFile().getAbsolutePath());
            }

            try {
                params.put("dest", pathDest.toFile().getCanonicalPath());
            } catch (IOException e) {
                params.put("dest", pathDest.toFile().getAbsolutePath());
            }
            HookHandler.doCheck(CheckParameter.Type.RENAME, params);
        }
    }

    /**
     * link hook；swap the source and destination ；logic same as rename
     *
     * @param pathSource 源文件路径
     * @param pathDest 目标文件路径
     */
    public static void checkFileLink(Path pathDest,Path pathSource) {
        boolean checkSwitch = Config.getConfig().getPluginFilter();
        if (pathSource != null && !pathSource.toFile().isDirectory() && pathDest != null && !pathDest.toFile().isDirectory()) {
            if (checkSwitch && !pathSource.toFile().exists()){
                return;
            }
            HashMap<String, Object> params = new HashMap<String, Object>();
            try {
                params.put("source", pathSource.toFile().getCanonicalPath());
            } catch (IOException e) {
                params.put("source", pathSource.toFile().getAbsolutePath());
            }

            try {
                params.put("dest", pathDest.toFile().getCanonicalPath());
            } catch (IOException e) {
                params.put("dest", pathDest.toFile().getAbsolutePath());
            }
            HookHandler.doCheck(CheckParameter.Type.RENAME, params);
        }
    }
}
