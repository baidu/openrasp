package com.baidu.openrasp.plugin.checker.policy.server;

import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.info.EventInfo;

import java.util.List;

/**
 * @description: BES security checker
 * @author: bes
 * @create: 2020/03/20
 */
public class BESSecurityChecker extends ServerPolicyChecker {

    public BESSecurityChecker() {
        super();
    }

    public BESSecurityChecker(boolean canBlock) {
        super(canBlock);
    }
    
	@Override
	public void checkServer(CheckParameter checkParameter, List<EventInfo> infos) {
		// TODO Auto-generated method stub

	}

}
