package com.baidu.openrasp.config;

/**
 * Created by tyy on 19-10-22.
 */
public abstract class ConfigSetter<T> {

    String itemName;

    public ConfigSetter(String itemName) {
        this.itemName = itemName;
    }

    abstract void setValue(T value);

    boolean setDefaultValue() {
        try {
            setValue(getDefaultValue());
        } catch (Throwable t) {
            Config.LOGGER.warn("set default value of " + itemName + " failed:" + t.getMessage(), t);
            return false;
        }
        return true;
    }

    abstract T getDefaultValue();
}
