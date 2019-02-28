package com.baidu.openrasp.plugin.antlr;

import org.antlr.v4.runtime.BaseErrorListener;
import org.antlr.v4.runtime.Lexer;
import org.antlr.v4.runtime.RecognitionException;
import org.antlr.v4.runtime.Recognizer;
import org.apache.log4j.Logger;


public class CmdTokenizeErrorListener extends BaseErrorListener {
    public static final Logger LOGGER = Logger.getLogger("com.fuxi.javaagent.plugin.js.engine.log");
    public static String LINE_SEP = System.getProperty("line.separator");

    @Override
    public void syntaxError(Recognizer<?, ?> recognizer, Object offendingSymbol, int line, int charPositionInLine, String msg, RecognitionException e) {
        LOGGER.info("RASP.cmd_tokenize() error: line " + line + ":" + charPositionInLine + " at " + offendingSymbol + ": " + msg + " in Command statement:" + LINE_SEP + ((Lexer) recognizer).getInputStream().toString());
    }
}
