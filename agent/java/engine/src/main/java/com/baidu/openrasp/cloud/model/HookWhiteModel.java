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
