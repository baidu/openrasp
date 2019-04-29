/*
 * Copyright 2017-2019 Baidu Inc.
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
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.info.AttackInfo;
import com.baidu.openrasp.plugin.info.EventInfo;

import java.util.*;

/**
 * Created by tyy on 17-12-20.
 * <p>
 * 检测 sql 语句的 java 版本
 */
public class SqlExceptionChecker extends ConfigurableChecker {

    private static final String CONFIG_KEY_SQL_EXCEPTION = "sql_exception";

    private static ArrayList<String> sqlErrorCode = new ArrayList<String>();

    static {
        sqlErrorCode.add("1060");// Duplicate column name '%s'
        sqlErrorCode.add("1062");// Duplicate entry '%s' for key %d
        sqlErrorCode.add("1105");// Unknown error
        sqlErrorCode.add("1367");// Illegal non geometric
        sqlErrorCode.add("1690");// BIGINT UNSIGNED value is out of range
        sqlErrorCode.add("1064");// %s near '%s' at line %d
        sqlErrorCode.add("1045");// Access denied for user '%s'@'%s' (using password: %s)
    }

    @Override
    public List<EventInfo> checkParam(CheckParameter checkParameter) {
        List<EventInfo> result = new LinkedList<EventInfo>();
        String action = getActionElement(Config.getConfig().getAlgorithmConfig(), CONFIG_KEY_SQL_EXCEPTION);
        // 检测sql执行报错注入
        if (!EventInfo.CHECK_ACTION_IGNORE.equals(action)) {
            String sqlType = (String) checkParameter.getParam("server");
            String errorCode = (String) checkParameter.getParam("error_code");
            if (sqlType != null && errorCode != null && sqlErrorCode.contains(errorCode)) {
                String message = (String) checkParameter.getParam("message");
                if (message != null) {
                    result.add(
                            AttackInfo.createLocalAttackInfo(checkParameter, action, message, "sql_exception", 90));
                }
            }
        }
        return result;
    }

}
