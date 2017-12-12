package com.fuxi.javaagent.exception;

/**
 * Created by tyy on 9/21/17.
 * 加载配置时发生的错误
 */
public class ConfigLoadException extends RuntimeException {

    public ConfigLoadException(Throwable cause) {
        super(cause);
    }

    /**
     * constructor
     *
     * @param message 加载配置异常信息
     */
    public ConfigLoadException(String message) {
        super(message);
    }

}
