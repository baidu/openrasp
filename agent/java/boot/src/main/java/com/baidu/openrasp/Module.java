package com.baidu.openrasp;

import java.lang.instrument.Instrumentation;

/**
 * Created by tyy on 18-2-1.
 *
 * 每个子模块入口都需要继承的模块
 * 模块入口类 配置在子模块 jar 包的 MANIFEST 配置中
 */
public interface Module {

    void start(String agentArg, Instrumentation inst) throws Exception;

    void release();

}
