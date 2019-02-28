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

package com.baidu.openrasp.cloud.model;

import com.baidu.openrasp.cloud.utils.DoubleArrayTrie;

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
}
