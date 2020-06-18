package com.baidu.openrasp.tool;

import org.yaml.snakeyaml.constructor.Constructor;

import java.util.HashSet;
import java.util.Set;

public class FilterConstructor extends Constructor {
    private static final Set<String> whiteClass = new HashSet<String>();
    static {
        whiteClass.add("java.util.Map");
    }
    @Override
    protected Class<?> getClassForName(String name) throws ClassNotFoundException {
        if (!whiteClass.contains(name)) {
            throw new ClassNotFoundException(String.format("Construct class '%s' is forbidden.", name));
        }
        return super.getClassForName(name);
    }
}
