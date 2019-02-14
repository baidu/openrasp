package com.baidu.openrasp;

import java.net.URL;
import java.net.URLClassLoader;
import java.net.URLStreamHandlerFactory;

/**
 * Created by tyy on 19-2-13.
 *
 * 模块类加载器
 */
public class MouduleClassLoader extends URLClassLoader {

    public MouduleClassLoader(URL[] urls) {
        super(urls);
    }

    public MouduleClassLoader(URL[] urls, ClassLoader parent) {
        super(urls, parent);
    }

    public MouduleClassLoader(URL[] urls, ClassLoader parent, URLStreamHandlerFactory factory) {
        super(urls, parent, factory);
    }

}
