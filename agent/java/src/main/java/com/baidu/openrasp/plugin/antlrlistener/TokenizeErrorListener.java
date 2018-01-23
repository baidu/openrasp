package com.baidu.openrasp.plugin.antlrlistener;

import org.antlr.v4.runtime.BaseErrorListener;
import org.antlr.v4.runtime.RecognitionException;
import org.antlr.v4.runtime.Recognizer;
import org.apache.log4j.Logger;

/**
 * Created by lxk on 1/22/18.
 */
public class TokenizeErrorListener extends BaseErrorListener {
    public static final Logger LOGGER = Logger.getLogger("com.fuxi.javaagent.plugin.js.engine.log");

    @Override
    public void syntaxError(Recognizer<?, ?> recognizer, Object offendingSymbol, int line, int charPositionInLine, String msg, RecognitionException e) {
        LOGGER.info("RASP.sql_tokenize error: line " + line + ":" + charPositionInLine + " at " + offendingSymbol + ": " + msg);
    }
}
