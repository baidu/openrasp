package com.baidu.openrasp.plugin.antlr;

/**
 * Created by lxk on 3/19/18.
 */
public class TokenResult {

    private String text;
    private int start, stop;

    public TokenResult(String tokenText, int startIndex, int stopIndex) {
        this.text = tokenText;
        this.start = startIndex;
        this.stop = stopIndex;
    }

    public String getText() {
        return this.text;
    }

    public int getStart() {
        return this.start;
    }

    public int getStop() {
        return this.stop;
    }
}
