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

package com.baidu.openrasp.tool;

import java.util.ArrayList;
import java.util.Collection;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * 　　* @Description: prepareStatement hook缓存
 * 　　* @author anyang
 * 　　* @date 2018/6/20 11:05
 *
 */
public class LRUCache<K, V> {

    private static final float hashTableLoadFactor = 0.75f;

    private LinkedHashMap<K, V> map;
    private int cacheSize;
    private final Lock lock = new ReentrantLock();

    /**
     * Creates a new LRU cache.
     *
     * @param cacheSize the maximum number of entries that will be kept in this cache.
     */
    public LRUCache(int cacheSize) {
        this.cacheSize = cacheSize;
        int hashTableCapacity = (int) Math.ceil(cacheSize / hashTableLoadFactor) + 1;
        map = new LinkedHashMap<K, V>(hashTableCapacity, hashTableLoadFactor, true) {
            private static final long serialVersionUID = 1;

            @Override
            protected boolean removeEldestEntry(Map.Entry<K, V> eldest) {
                return size() > LRUCache.this.cacheSize;
            }
        };
    }

    /**
     * Retrieves an entry from the cache.<br>
     * The retrieved entry becomes the MRU (most recently used) entry.
     *
     * @param key the key whose associated value is to be returned.
     * @return the value associated to this key, or null if no value with this key exists in the cache.
     */
    public V get(K key) {

        try {
            lock.lock();
            return map.get(key);
        } finally {
            lock.unlock();
        }

    }

    /**
     * Adds an entry to this cache.
     * The new entry becomes the MRU (most recently used) entry.
     * If an entry with the specified key already exists in the cache, it is replaced by the new entry.
     * If the cache is full, the LRU (least recently used) entry is removed from the cache.
     *
     * @param key   the key with which the specified value is to be associated.
     * @param value a value to be associated with the specified key.
     */
    public void put(K key, V value) {
        try {
            lock.lock();
            map.put(key, value);
        } finally {
            lock.unlock();
        }
    }

    /**
     * Clears the cache.
     */
    public synchronized void clear() {

        try {
            lock.lock();
            map.clear();
        } finally {
            lock.unlock();
        }
    }

    /**
     * Returns the existion of  in the cache.
     */
    public boolean isContainsKey(K key) {

        try {
            lock.lock();
            return map.containsKey(key);
        } finally {
            lock.unlock();
        }
    }

    /**
     * Returns a <code>Collection</code> that contains a copy of all cache entries.
     *
     * @return a <code>Collection</code> with a copy of the cache content.
     */
    public Collection<Map.Entry<K, V>> getAll() {

        try {
            lock.lock();
            return new ArrayList<Map.Entry<K, V>>(map.entrySet());
        } finally {
            lock.unlock();
        }
    }
}
