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

package com.baidu.openrasp.messaging;

import org.apache.log4j.spi.Filter;
import org.apache.log4j.spi.LoggingEvent;


/**
 * Created by lxk on 17-4-10.
 * 日志限速过滤器
 */
public class BurstFilter extends Filter {

    private long refillAmount;
    private long refillInterval;
    private long maxBurst;
    private TokenBucket tokenBucket;

    public long getRefillAmount() {
        return refillAmount;
    }

    public void setRefillAmount(long refillAmount) {
        this.refillAmount = refillAmount;
    }

    public long getRefillInterval() {
        return refillInterval;
    }

    public void setRefillInterval(long refillInterval) {
        this.refillInterval = refillInterval;
    }

    public long getMaxBurst() {
        return maxBurst;
    }

    public void setMaxBurst(long maxBurst) {
        this.maxBurst = maxBurst;
    }

    static class TokenBucket {

        private long refillAmount;
        private long refillInterval;
        private long capacity;
        private long currentTokenAmount;
        private long lastConsumedTime;

        /**
         *
         * @param refillAmount
         * @param refillInterval
         * @param capacity
         */
        public TokenBucket(long refillAmount, long refillInterval, long capacity) {
            this.refillAmount = refillAmount;
            this.refillInterval = refillInterval;
            this.capacity = capacity;
            this.currentTokenAmount = capacity;
            lastConsumedTime = System.currentTimeMillis();
        }

        /**
         * 消费一个Token
         *
         * @return
         */
        public synchronized boolean consume() {
            refill();
            boolean isEmpty = currentTokenAmount <= 0;

            if (!isEmpty) {
                currentTokenAmount--;
                lastConsumedTime = System.currentTimeMillis();
            }

            return isEmpty;
        }

        /**
         * 重新分配Token
         */
        private void refill() {
            long currentTime = System.currentTimeMillis();
            long elapsedTimeFromLastConsumed = (long) ((currentTime / 1000) - (lastConsumedTime / 1000));

            if (elapsedTimeFromLastConsumed >= refillInterval) {
                long refillTokenAmount = (elapsedTimeFromLastConsumed / refillInterval)
                        * refillAmount;
                currentTokenAmount = currentTokenAmount + refillTokenAmount > capacity ? capacity :
                        currentTokenAmount + refillTokenAmount;
            }
        }
    }

    /**
     *
     * @param event 需进行裁决的loggingEvent
     * @return 裁决结果
     */
    public int decide(LoggingEvent event) {
        if (tokenBucket == null) {
            tokenBucket = new TokenBucket(refillAmount, refillInterval,
                    maxBurst);
        }

        return tokenBucket.consume() ? Filter.DENY : Filter.NEUTRAL;
    }
}
