package com.baidu.openrasp.plugin.checker.v8;

import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.checker.policy.PolicyChecker;
import com.baidu.openrasp.plugin.info.EventInfo;
import com.baidu.openrasp.plugin.js.JS;

import java.util.List;

/**
 * Created by tyy on 19-12-5.
 *
 * V8 基线检测
 */
public class V8PolicyChecker extends PolicyChecker {

    public V8PolicyChecker() {
        super();
    }

    public V8PolicyChecker(boolean canBlock) {
        super(canBlock);
    }

    @Override
    public List<EventInfo> checkParam(CheckParameter checkParameter) {
        return JS.Check(checkParameter);
    }
}
