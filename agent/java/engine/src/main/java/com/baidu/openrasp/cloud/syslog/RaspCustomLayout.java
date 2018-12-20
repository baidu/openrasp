package com.baidu.openrasp.cloud.syslog;

import org.apache.log4j.PatternLayout;
import org.apache.log4j.helpers.PatternParser;

/**
 * @program openrasp
 * @description: 自定义patternLayout
 * @author: anyang
 * @create: 2018/10/23 11:49
 */
public class RaspCustomLayout extends PatternLayout {
    public RaspCustomLayout() {
    }

    public RaspCustomLayout(String pattern) {
        super(pattern);
    }

    @Override
    protected PatternParser createPatternParser(String pattern) {
        return new ExtPatternParser(pattern);
    }

}
