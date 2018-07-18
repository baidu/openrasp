package com.baidu.openrasp.hook.server.jboss;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.Reflection;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Created by anyang on 2018/6/14.
 */
public class JbossOutputBufferFlushHook extends AbstractClassHook {
    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/catalina/connector/OutputBuffer".equals(className);
    }

    @Override
    public String getType() {
        return "xss";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {

        String src=getInvokeStaticSrc(JbossOutputBufferFlushHook.class,"getJbossOutputBuffer","$0", Object.class);
        insertBefore(ctClass, "close", "()V", src);

    }

    public static void getJbossOutputBuffer(Object object){

            int parameterLength = Integer.valueOf(Config.getConfig().getXssParameterLength());
            String regex = Config.getConfig().getXssRegex();
            try {
                Object buffer= Reflection.getField(object,"bb");
                String content=new String(buffer.toString().getBytes(),"utf-8");
                if (content.contains("<html>")&&!content.contains("Content-Length")){
                    Map<String,String> serverInfo= HookHandler.requestCache.get().getServerContext();
                    if (serverInfo.get("server").contains("JBoss")){
                        HashMap<String, Object> params = null;
                        Map<String, String[]> parameterMap = HookHandler.requestCache.get().getParameterMap();
                        int exceedLengthCount = 0;
                        List<String> paramList = new ArrayList<String>();
                        for (Map.Entry<String, String[]> entry : parameterMap.entrySet()) {

                            for (String value : entry.getValue()) {
                                Pattern pattern = Pattern.compile(regex);
                                Matcher matcher = pattern.matcher(value);
                                boolean isMatch = matcher.find();
                                if (value.length() >= parameterLength && isMatch) {
                                    exceedLengthCount++;
                                    paramList.add(value);
                                }
                            }
                        }

                        params = new HashMap<String, Object>(3);
                        params.put("exceed_count", exceedLengthCount);
                        params.put("html_body", content);
                        params.put("param_list", paramList);
                        HookHandler.doCheck(CheckParameter.Type.XSS, params);
                    }

                }

            } catch (Exception e) {

                e.printStackTrace();
            }


    }

}
