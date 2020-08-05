package com.baidu.openrasp.messaging;

import com.baidu.openrasp.config.Config;
import org.apache.log4j.Logger;

/**
 * Created by tyy on 19-7-17.
 */
public class LogTool {

    public static Logger LOGGER = Logger.getLogger(LogTool.class);

    public static void traceWarn(ErrorType errorType, String message, Throwable t) {
        if (Config.getConfig().isDebugEnabled()) {
            warn(errorType, message, t);
        }
    }

    public static void traceError(ErrorType errorType, String message, Throwable t) {
        if (Config.getConfig().isDebugEnabled()) {
            error(errorType, message, t);
        }
    }

    public static void traceInfo(String message) {
        if (Config.getConfig().isDebugEnabled()) {
            LOGGER.info(message);
        }
    }

    public static void traceWarn(ErrorType errorType, String message) {
        if (Config.getConfig().isDebugEnabled()) {
            warn(errorType, message);
        }
    }

    public static void traceError(ErrorType errorType, String message) {
        if (Config.getConfig().isDebugEnabled()) {
            error(errorType, message);
        }
    }

    public static void traceHookWarn(String message, Throwable t) {
        traceError(ErrorType.HOOK_ERROR, message, t);
    }

    public static void warn(ErrorType errorType, String message, Throwable t) {
        LOGGER.warn(new ExceptionModel(errorType, message), t);
    }

    public static void error(ErrorType errorType, String message, Throwable t) {
        LOGGER.error(new ExceptionModel(errorType, message), t);
    }

    public static void warn(ErrorType errorType, String message) {
        LOGGER.warn(new ExceptionModel(errorType, message));
    }

    public static void error(ErrorType errorType, String message) {
        LOGGER.error(new ExceptionModel(errorType, message));
    }

}
