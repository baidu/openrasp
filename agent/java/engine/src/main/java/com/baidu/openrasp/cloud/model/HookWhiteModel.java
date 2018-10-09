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

package com.baidu.openrasp.cloud.model;

import com.baidu.openrasp.cloud.Utils.DoubleArrayTrie;

import java.util.*;

/**
 * @description: 缓存hook点的白名单信息
 * @author: anyang
 * @create: 2018/09/13 20:55
 */
public class HookWhiteModel {
    private static HashMap<String, Object> hookWhiteinfo = new HashMap<String, Object>();

    public static void init(String type, ArrayList<String> hooks) {
        if (!hooks.isEmpty()) {
            if (hooks.contains("all")) {
                hookWhiteinfo.put(type, "all");
            } else {
                DoubleArrayTrie dat = new DoubleArrayTrie();
                Collections.sort(hooks);
                dat.build(hooks);
                hookWhiteinfo.put(type, dat);
            }
        }

    }

    public static HashMap<String, Object> getHookWhiteInfo() {
        return hookWhiteinfo;
    }

    public static boolean isContainURL(String type, String url) {
        Object object = hookWhiteinfo.get(type);
        if (object instanceof String && "all".equals(String.valueOf(object))) {
            return true;
        } else if (object instanceof DoubleArrayTrie) {
            List<Integer> list = ((DoubleArrayTrie) object).commonPrefixSearch(url);
            return !list.isEmpty();
        }
        return false;
    }
}
