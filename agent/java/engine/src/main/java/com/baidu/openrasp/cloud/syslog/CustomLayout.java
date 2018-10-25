package com.baidu.openrasp.cloud.syslog;

import com.baidu.openrasp.config.Config;
import org.apache.log4j.PatternLayout;
import org.apache.log4j.spi.LoggingEvent;

/**
 * @program openrasp
 * @description: ddd
 * @author: anyang
 * @create: 2018/10/25 14:19
 */
public class CustomLayout extends PatternLayout {
    @Override
    public String format(LoggingEvent event) {
        return Config.getConfig().getSyslogTag();
    }
}
