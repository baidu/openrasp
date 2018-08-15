package com.baidu.openrasp.hook.server;

import com.baidu.openrasp.hook.AbstractClassHook;

/**
 * @author anyang
 * @Description: xss检测基类
 * @date 2018/8/15 15:37
 */
public abstract class ServerXssHook extends AbstractClassHook {
    @Override
    public String getType() {
        return "xss";
    }
}
