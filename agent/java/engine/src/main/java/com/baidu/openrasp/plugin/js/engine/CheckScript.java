/*
 * Copyright 2017-2018 Baidu Inc.
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

package com.baidu.openrasp.plugin.js.engine;

import com.google.gson.Gson;
import org.apache.commons.io.FileUtils;

import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

/**
 * Created by tyy on 4/6/17.
 * All rights reserved
 */
public class CheckScript {

    private final String name;
    private final String content;

    public CheckScript(File file) throws IOException {
        this(file.getName(), FileUtils.readFileToString(file, "UTF-8"));
    }

    public CheckScript(String name, String content) {
        this.name = name;
        this.content = content;
    }

    @Override
    public boolean equals(Object obj) {
        if (this.getClass().isInstance(obj)) {
            CheckScript other = (CheckScript) obj;
            return other.getName().equals(this.getName()) && other.getContent().equals(this.getContent());
        }
        return false;
    }

    @Override
    public int hashCode() {
        int result = 17;
        result = 131 * result + (this.name != null ? this.name.hashCode() : 0);
        result = 131 * result + (this.content != null ? this.content.hashCode() : 0);
        return result;
    }

    public String getName() {
        return name;
    }

    public String getContent() {
        return content;
    }

    @Override
    public String toString() {
        Map<String, Object> obj = new HashMap<String, Object>();
        obj.put("name", name);
        obj.put("content", content);
        return new Gson().toJson(obj);
    }
}
