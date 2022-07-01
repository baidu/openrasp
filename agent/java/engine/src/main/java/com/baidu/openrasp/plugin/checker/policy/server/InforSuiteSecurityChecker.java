package com.baidu.openrasp.plugin.checker.policy.server;

import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.info.EventInfo;

import java.util.List;

/**
 * @description: InforSuite security checker
 * @author: inforsuite
 * @create: 2020/03/20
 */
public class InforSuiteSecurityChecker extends ServerPolicyChecker {

    public InforSuiteSecurityChecker() {
        super();
    }

    public InforSuiteSecurityChecker(boolean canBlock) {
        super(canBlock);
    }
    
	@Override
	public void checkServer(CheckParameter checkParameter, List<EventInfo> infos) {

	}

}
