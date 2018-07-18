/*
 * Copyright 2017-2018 Baidu Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.baidu.openrasp.plugin.checker.local;

import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.plugin.checker.AttackChecker;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.info.AttackInfo;
import com.baidu.openrasp.plugin.info.EventInfo;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

/**
　　* @Description: XSS检测
　　* @author anyang
　　* @date 2018/6/22 11:26
　　*/
public class XssChecker extends AttackChecker {

    public XssChecker() {
        super();
    }

    public XssChecker(boolean canBlock) {
        super(canBlock);
    }

    @Override
    public List<EventInfo> checkParam(CheckParameter checkParameter) {
        LinkedList<EventInfo> result = new LinkedList<EventInfo>();
        Integer exceedCount=(Integer) checkParameter.getParam("exceed_count");
        @SuppressWarnings("unchecked")
        List<String> paramList=(ArrayList<String>)checkParameter.getParam("param_list");
        String content=String.valueOf(checkParameter.getParam("html_body"));
        if (exceedCount != null) {
            Integer exceedLengthCount = Integer.valueOf(Config.getConfig().getXssExceedLengthCount());
            Integer xssParameterLength = Integer.valueOf(Config.getConfig().getXssParameterLength());
            if (exceedCount >= exceedLengthCount) {
                result.add(AttackInfo.createLocalAttackInfo(checkParameter, EventInfo.CHECK_ACTION_BLOCK, "所有的请求参数中长度大于等于"+xssParameterLength+"并且匹配XSS正则的数量超过了" + exceedLengthCount));
                return result;
            }
        }
        if (content!=null&&!paramList.isEmpty()){
            for (String param:paramList){

                if (content.contains(param)){
                    result.add(AttackInfo.createLocalAttackInfo(checkParameter, EventInfo.CHECK_ACTION_BLOCK, "请求参数"+param+"存在XSS攻击风险"));
                    return result;
                }
            }
        }

        return result;
    }
}
