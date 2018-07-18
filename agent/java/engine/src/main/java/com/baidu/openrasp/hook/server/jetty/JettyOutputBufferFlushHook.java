package com.baidu.openrasp.hook.server.jetty;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.Reflection;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * 　　* @Description: jetty 获取输出buffer的hook点
 * 　　* @author anyang
 * 　　* @date 2018/6/11 14:53
 */
public class JettyOutputBufferFlushHook extends AbstractClassHook {

    @Override
    public boolean isClassMatched(String className) {
        return "org/eclipse/jetty/server/AbstractHttpConnection".equals(className);
    }

    @Override
    public String getType() {
        return "xss";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {

        String src = getInvokeStaticSrc(JettyOutputBufferFlushHook.class, "getJettyOutputBuffer", "_generator", Object.class);
        insertBefore(ctClass, "completeResponse", "()V", src);

    }


    public static void getJettyOutputBuffer(Object object) {

        boolean isEnableXssHook = HookHandler.isEnableXssHook();
        if (isEnableXssHook) {
            HookHandler.disableXssHook();
            int parameterLength = Integer.valueOf(Config.getConfig().getXssParameterLength());
            String regex = Config.getConfig().getXssRegex();

            try {
                Object buffer = Reflection.getSuperField(object, "_buffer");
                String content = new String(buffer.toString().getBytes(), "utf-8");
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
