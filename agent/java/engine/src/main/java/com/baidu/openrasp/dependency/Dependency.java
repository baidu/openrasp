package com.baidu.openrasp.dependency;

import java.util.Arrays;

/**
 * @description: 保存依赖信息类
 * @author: anyang
 * @create: 2019/04/19 12:03
 */
public class Dependency{
    public final String name;
    public final String version;
    public final String location;

    public Dependency(String name, String version, String location) {
        this.name = name;
        this.version = version;
        this.location = location;
    }

    @Override
    public int hashCode() {
        return Arrays.hashCode(new Object[]{this.name, this.version, this.location});
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) return true;
        if (obj == null || getClass() != obj.getClass()) return false;
        Dependency dependency = (Dependency) obj;
        if (!name.equals(dependency.name)) return false;
        if (!version.equals(dependency.version)) return false;
        return location.equals(dependency.location);
    }

    @Override
    public String toString() {
        return "Dependency{" +
                "name='" + name + '\'' +
                ", version='" + version + '\'' +
                ", location='" + location + '\'' +
                '}';
    }
}
