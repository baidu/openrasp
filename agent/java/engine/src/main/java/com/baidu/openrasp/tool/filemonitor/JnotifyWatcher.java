package com.baidu.openrasp.tool.filemonitor;

import com.fuxi.javaagent.contentobjects.jnotify.Observer;
import org.apache.log4j.Logger;

/**
 * Created by anyang on 2018/5/4.
 */
public class JnotifyWatcher implements Observer {

    private static final Logger LOGGER = Logger.getLogger(JnotifyWatcher.class.getName());
    public String message;

    @Override
    public void update(String s) {
        this.message=s;
        LOGGER.info("JNotify encounters an internal error :"+message);
    }
}
