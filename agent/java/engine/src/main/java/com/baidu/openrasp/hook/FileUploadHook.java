package com.baidu.openrasp.hook;

import com.baidu.openrasp.tool.Reflection;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;
import java.util.HashMap;
import java.util.List;

/**
 * Created by anyang on 2018/7/5.
 */
public class FileUploadHook extends AbstractClassHook {

    public static HashMap<String,String[]> fileUploadCache = new HashMap<String, String[]>();

    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/commons/fileupload/FileUploadBase".equals(className);
    }

    @Override
    public String getType() {
        return null;
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {

        String src = getInvokeStaticSrc(FileUploadHook.class, "cacheFileUploadParam", "$_", Object.class);
        insertAfter(ctClass, "parseRequest", null, src);

    }

    public static void cacheFileUploadParam(Object object) {

        List<Object> list = (List<Object>) object;
        if (!list.isEmpty()){
            fileUploadCache.clear();
            for (Object o : list) {
                boolean isFormField = (Boolean) Reflection.invokeMethod(o, "isFormField", new Class[]{}, null);
                if (isFormField) {
                    String fieldName = Reflection.invokeStringMethod(o, "getFieldName", new Class[]{}, null);
                    String fieldValue = Reflection.invokeStringMethod(o, "getString", new Class[]{}, null);
                    fileUploadCache.put(fieldName, new String[]{fieldValue});
                    System.out.println(fieldName + "====" + fieldValue);
                } else {
                    String fileName = Reflection.invokeStringMethod(o, "getName", new Class[]{}, null);
                    String fileContent = Reflection.invokeStringMethod(o, "getString", new Class[]{}, null);
                    try {
                       fileContent=new String(fileContent.getBytes(GetFileUploadCharsetHook.charset), "gbk");
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    fileUploadCache.put(fileName, new String[]{fileContent});
                    System.out.println(fileName + "===" + fileContent);
                }
            }
        }

    }
}
