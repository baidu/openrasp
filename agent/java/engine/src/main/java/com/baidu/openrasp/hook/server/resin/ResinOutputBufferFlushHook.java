package com.baidu.openrasp.hook.server.resin;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


/**
 * 　　* @Description: resin 获取输出buffer的hook点
 * 　　* @author anyang
 * 　　* @date 2018/6/11 14:53
 *
 */
public class ResinOutputBufferFlushHook extends AbstractClassHook {

    @Override
    public boolean isClassMatched(String className) {
        return "com/caucho/server/http/ToByteResponseStream".equals(className)||
                "com/caucho/server/connection/ToByteResponseStream".equals(className);
    }

    @Override
    public String getType() {
        return "xss";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {

        String src = getInvokeStaticSrc(ResinOutputBufferFlushHook.class, "getResinOutputBuffer", "_charBuffer,_charLength,_isOutputStreamOnly", char[].class, int.class, boolean.class);
        insertBefore(ctClass, "flushCharBuffer", "()V", src);

    }

    public static void getResinOutputBuffer(char[] buffer, int len, boolean isOutputStreamOnly) {


        int parameterLength = Integer.valueOf(Config.getConfig().getXssParameterLength());
        String regex = Config.getConfig().getXssRegex();
        if (len > 0 && !isOutputStreamOnly) {
            char[]temp=new char[len];
            System.arraycopy(buffer,0,temp,0,len);
            String content = new String(temp);
            try {
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

                } catch (Exception e) {

                    e.printStackTrace();
                }
        }


    }
}
