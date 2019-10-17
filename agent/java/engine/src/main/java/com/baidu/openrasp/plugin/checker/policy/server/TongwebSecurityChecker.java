package com.baidu.openrasp.plugin.checker.policy.server;

import java.util.List;

import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.info.EventInfo;

public class TongwebSecurityChecker extends ServerPolicyChecker {

    public TongwebSecurityChecker() {
        super();
    }

    public TongwebSecurityChecker(boolean canBlock) {
        super(canBlock);
    }
    
	@Override
	public void checkServer(CheckParameter checkParameter, List<EventInfo> infos) {
		// TODO Auto-generated method stub

	}

}
