package com.baidu.openrasp.plugin.antlr;

import com.baidu.openrasp.Bash;
import com.baidu.openrasp.SQLLexer;
import org.antlr.v4.runtime.ANTLRErrorListener;
import org.antlr.v4.runtime.ANTLRInputStream;
import org.antlr.v4.runtime.Token;

import java.util.ArrayList;

/**
 * Created by lxk on 3/19/18.
 */
public class TokenGenerator {

    /**
     * tokenize query clause
     *
     * @param text query clause
     * @return tokens
     */
    public static String[] tokenize(String text) {
        return tokenize(text, null);
    }

    /**
     * tokenize query clause
     *
     * @param text     query clause
     * @param listener antlr error listener
     * @return tokens
     */
    public static String[] tokenize(String text, ANTLRErrorListener listener) {
        ANTLRInputStream input = new ANTLRInputStream(text);
        SQLLexer lexer = new SQLLexer(input);
        if (listener != null) {
            lexer.removeErrorListeners();
            lexer.addErrorListener(listener);
        }
        ArrayList<String> result = new ArrayList<String>();
        for (Token token = lexer.nextToken();
             token.getType() != Token.EOF;
             token = lexer.nextToken()) {
            result.add(token.getText());
        }
        return result.toArray(new String[0]);
    }

    /**
     * tokenize query clause
     *
     * @param text     query clause
     * @param listener antlr error listener
     * @return TokenResults
     */
    public static ArrayList<TokenResult> detailTokenize(String text, ANTLRErrorListener listener) {
        ANTLRInputStream input = new ANTLRInputStream(text);
        SQLLexer lexer = new SQLLexer(input);
        if (listener != null) {
            lexer.removeErrorListeners();
            lexer.addErrorListener(listener);
        }
        ArrayList<TokenResult> result = new ArrayList<TokenResult>();
        for (Token token = lexer.nextToken();
             token.getType() != Token.EOF;
             token = lexer.nextToken()) {
            result.add(new TokenResult(token.getText(), token.getStartIndex(), token.getStopIndex()));
        }
        return result;
    }

    /**
     * tokenize query clause
     *
     * @param text     query clause
     * @param listener antlr error listener
     * @return Tokens
     */
    public static ArrayList<Token> rawTokenize(String text, ANTLRErrorListener listener) {
        ANTLRInputStream input = new ANTLRInputStream(text);
        SQLLexer lexer = new SQLLexer(input);
        if (listener != null) {
            lexer.removeErrorListeners();
            lexer.addErrorListener(listener);
        }
        ArrayList<Token> result = new ArrayList<Token>();
        for (Token token = lexer.nextToken();
             token.getType() != Token.EOF;
             token = lexer.nextToken()) {
            result.add(token);
        }
        return result;
    }

    /**
     * tokenize query clause
     *
     * @param text     query clause
     * @param listener antlr error listener
     * @return TokenResults
     */
    public static ArrayList<TokenResult> cmdTokenize(String text, ANTLRErrorListener listener) {
        ANTLRInputStream input = new ANTLRInputStream(text);
        Bash lexer = new Bash(input);
        if (listener != null) {
            lexer.removeErrorListeners();
            lexer.addErrorListener(listener);
        }
        ArrayList<TokenResult> result = new ArrayList<TokenResult>();
        for (Token token = lexer.nextToken();
             token.getType() != Token.EOF;
             token = lexer.nextToken()) {
            result.add(new TokenResult(token.getText(), token.getStartIndex(), token.getStopIndex()));
        }
        return result;
    }
}
