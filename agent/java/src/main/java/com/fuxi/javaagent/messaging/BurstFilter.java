/**
 * Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

package com.fuxi.javaagent.messaging;

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
