package com.baidu.openrasp.plugin.checker.policy.server;

import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.info.EventInfo;
import java.util.List;

/**
 * @description: InforSuiteAS ServerPolicyChecker
 * @author: codff
 * @create: 2022/03/17
 */
public class InforSuiteASSecurityChecker extends ServerPolicyChecker {

    public InforSuiteASSecurityChecker() {
        super();
    }

    public InforSuiteASSecurityChecker(boolean canBlock) {
        super(canBlock);
    }

    @Override
    public void checkServer(CheckParameter checkParameter, List<EventInfo> infos) {
        // TODO Auto-generated method stub

    }
}
