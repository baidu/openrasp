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

package com.baidu.openrasp.tool.decomplie;

import com.strobel.assembler.InputTypeLoader;
import com.strobel.assembler.metadata.Buffer;
import com.strobel.assembler.metadata.ITypeLoader;

import java.util.ArrayList;
import java.util.List;

/**
 * @author: anyang
 * @create: 2018/10/18 20:47
 */
public class RaspTypeLoader implements ITypeLoader {
    private final List<ITypeLoader> _typeLoaders;

    public RaspTypeLoader() {
        _typeLoaders = new ArrayList<ITypeLoader>();
        _typeLoaders.add(new InputTypeLoader());
    }

    public final List<ITypeLoader> getTypeLoaders() {
        return _typeLoaders;
    }

    public boolean tryLoadType(final String internalName, final Buffer buffer) {
        for (final ITypeLoader typeLoader : _typeLoaders) {
            if (typeLoader.tryLoadType(internalName, buffer)) {
                return true;
            }

            buffer.reset();
        }

        return false;
    }
}
