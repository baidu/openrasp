package com.baidu.openrasp.cloud.syslog;

import com.baidu.openrasp.config.Config;
import org.apache.log4j.helpers.PatternConverter;
import org.apache.log4j.helpers.PatternParser;
import org.apache.log4j.spi.LoggingEvent;

/**
 * @description: 自定义log4j的pattern解析器
 * @author: anyang
 * @create: 2018/10/23 17:02
 */
public class ExtPatternParser extends PatternParser {

    private static final char SYSLOG_TAG = 'e';

    public ExtPatternParser(String pattern) {
        super(pattern);
    }

    @Override
    protected void finalizeConverter(char c) {

        switch (c) {
            case SYSLOG_TAG:
                currentLiteral.setLength(0);
                addConverter(new ExtPatternConverter());
                break;
            default:
                super.finalizeConverter(c);
                break;
        }
    }

    private class ExtPatternConverter extends PatternConverter {
        @Override
        public String convert(LoggingEvent event) {
            return Config.getConfig().getSyslogTag();
        }
    }

}
