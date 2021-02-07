/*
 * Copyright 2017-2021 Baidu Inc.
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

package com.baidu.openrasp.cloud.model;

import com.baidu.openrasp.cloud.utils.DoubleArrayTrie;
import com.baidu.openrasp.plugin.checker.CheckParameter;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * @description: 缓存hook点的白名单信息
 * @author: anyang
 * @create: 2018/09/13 20:55
 */
public class HookWhiteModel {
    private static DoubleArrayTrie hookWhiteinfo;
    private static ReentrantReadWriteLock lock = new ReentrantReadWriteLock();

    public static void init(TreeMap<String, Integer> urls) {
        DoubleArrayTrie temp = null;
        if (!urls.isEmpty()) {
            temp = new DoubleArrayTrie();
            List<String> list = new ArrayList<String>(urls.size());
            int[] value = new int[urls.size()];
            int index = 0;
            for (Map.Entry<String, Integer> entry : urls.entrySet()) {
                list.add(entry.getKey());
                value[index++] = entry.getValue();
            }
            temp.build(list, value);
        }
        try {
            lock.writeLock().lock();
            hookWhiteinfo = temp;
        } finally {
            lock.writeLock().unlock();
        }
    }

    public static boolean isContainURL(Integer code, String url) {
        if (hookWhiteinfo != null) {
            List<Integer> matched;
            try {
                lock.readLock().lock();
                matched = hookWhiteinfo.commonPrefixSearch(url);
            } finally {
                lock.readLock().unlock();
            }
            if (matched != null && !matched.isEmpty()) {
                int result = 0;
                for (Integer i : matched) {
                    result = result | i;
                }
                return (code & result) != 0;
            }
        }
        return false;
    }

    public static TreeMap<String, Integer> parseHookWhite(Map<Object, Object> hooks) {
        TreeMap<String, Integer> temp = new TreeMap<String, Integer>();
        for (Map.Entry<Object, Object> hook : hooks.entrySet()) {
            int codeSum = 0;
            if (hook.getValue() instanceof ArrayList) {
                ArrayList<String> types = (ArrayList<String>) hook.getValue();
                if (hook.getKey().equals("*") && types.contains("all")) {
                    for (CheckParameter.Type type : CheckParameter.Type.values()) {
                        if (type.getCode() != 0) {
                            codeSum = codeSum + type.getCode();
                        }
                    }
                    temp.put("", codeSum);
                    return temp;
                } else if (types.contains("all")) {
                    for (CheckParameter.Type type : CheckParameter.Type.values()) {
                        if (type.getCode() != 0) {
                            codeSum = codeSum + type.getCode();
                        }
                    }
                    temp.put(hook.getKey().toString(), codeSum);
                } else {
                    for (String s : types) {
                        String hooksType = s.toUpperCase();
                        try {
                            Integer code = CheckParameter.Type.valueOf(hooksType).getCode();
                            codeSum = codeSum + code;
                        } catch (Exception e) {
//                            LogTool.traceWarn(ErrorType.CONFIG_ERROR, "Hook type " + s + " does not exist", e);
                        }
                    }
                    if (hook.getKey().equals("*")) {
                        temp.put("", codeSum);
                    } else {
                        temp.put(hook.getKey().toString(), codeSum);
                    }
                }
            }
        }
        return temp;
    }
}
