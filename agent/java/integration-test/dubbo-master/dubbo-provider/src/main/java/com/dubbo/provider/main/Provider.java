package com.dubbo.provider.main;

import org.springframework.context.support.ClassPathXmlApplicationContext;

import java.io.IOException;

/**
 * Created by anyang on 2018/6/25.
 */
public class Provider {

    static int count=0;
    public static void main(String[] args) throws IOException {
        count++;
        ClassPathXmlApplicationContext context = new ClassPathXmlApplicationContext(new String[]{"applicationContext-provider.xml"});
        context.start();
        System.in.read();

    }
}
