/*
 * Copyright 2017-2021 Baidu Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.baidu.openrasp.dependency;

import java.util.Arrays;

/**
 * @description: 保存依赖信息类
 * @author: anyang
 * @create: 2019/04/19 12:03
 */
public class Dependency {
    public final String product;
    public final String version;
    public final String vendor;
    public final String path;
    public final String source;

    public Dependency(String product, String version, String vendor, String path, String source) {
        this.product = product;
        this.version = version;
        this.vendor = vendor;
        this.path = path;
        this.source = source;
    }

    @Override
    public int hashCode() {
        return Arrays.hashCode(new Object[]{this.product, this.version, this.vendor, this.path});
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) return true;
        if (obj == null || getClass() != obj.getClass()) return false;
        Dependency dependency = (Dependency) obj;
        if (!product.equals(dependency.product)) return false;
        if (!version.equals(dependency.version)) return false;
        if (!vendor.equals(dependency.vendor)) return false;
        return path.equals(dependency.path);
    }

    @Override
    public String toString() {
        return "Dependency{" +
                "name='" + product + '\'' +
                ", version='" + version + '\'' +
                ", vendor='" + vendor + '\'' +
                ", location='" + path + '\'' +
                '}';
    }
}
