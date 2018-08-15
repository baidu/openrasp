package com.baidu.openrasp.hook.server;

import com.baidu.openrasp.hook.AbstractClassHook;

/**
 * @author anyang
 * @Description: server xss 基础类
 * @date 2018/8/15 14:19
 */
public abstract class ServerXssHook extends AbstractClassHook {
    @Override
    public String getType() {
        return "xss";
    }
}
