package com.fuxi.javaagent.tool.security;

import org.apache.log4j.Logger;

/**
 * Created by tyy on 17-11-14.
 */
public interface PolicyChecker {

    public static final Logger ALARM_LOGGER = Logger.getLogger(PolicyChecker.class.getPackage().getName() + ".policy_alarm");

    boolean check();

}
