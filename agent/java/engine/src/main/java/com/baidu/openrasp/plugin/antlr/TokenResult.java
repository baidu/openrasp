package com.baidu.openrasp.plugin.antlr;

/**
 * Created by lxk on 3/19/18.
 */
public class TokenResult {

    private int type;
    private String text;
    private int startIndex;

    public TokenResult(int type, String text, int startIndex)
    {
        this.type = type;
        this.text = text;
        this.startIndex = startIndex;
    }

    public int getType()
    {
        return this.type;
    }

    public String getText()
    {
        return this.text;
    }

    public int getStartIndex()
    {
        return this.startIndex;
    }
}
