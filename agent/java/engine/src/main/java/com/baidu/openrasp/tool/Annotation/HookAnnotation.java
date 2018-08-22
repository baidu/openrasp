package com.baidu.openrasp.tool.Annotation;

import java.lang.annotation.*;

/**
 * @author anyang
 * @Description: hook点标识
 * @date 2018/8/16 10:12
 */
@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
@Documented
public @interface HookAnnotation {

}
