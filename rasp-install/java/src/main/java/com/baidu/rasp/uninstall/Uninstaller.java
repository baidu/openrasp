package com.baidu.rasp.uninstall;

import com.baidu.rasp.RaspError;

import java.io.IOException;

/**
　　* @Description: 自动卸载接口
　　* @author anyang
　　* @date 2018/4/25 19:37
　　*/
public interface Uninstaller {

    void uninstall() throws RaspError, IOException;
}
