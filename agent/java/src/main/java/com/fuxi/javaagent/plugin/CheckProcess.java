package com.fuxi.javaagent.plugin;

import org.mozilla.javascript.Function;

public class CheckProcess {
    private Function function;
    private String pluginName;

    public CheckProcess(Function function, String pluginName) {
        this.function = function;
        this.pluginName = pluginName;
    }

    public Function getFunction() {
        return function;
    }

    public void setFunction(Function function) {
        this.function = function;
    }

    public String getPluginName() {
        return pluginName;
    }

    public void setPluginName(String pluginName) {
        this.pluginName = pluginName;
    }
}
