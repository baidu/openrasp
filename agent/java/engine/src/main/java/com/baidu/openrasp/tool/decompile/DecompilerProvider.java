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

package com.baidu.openrasp.tool.decompile;

import com.strobel.assembler.metadata.TypeDefinition;
import com.strobel.decompiler.DecompilationOptions;
import com.strobel.decompiler.DecompilerSettings;
import com.strobel.decompiler.PlainTextOutput;

import java.io.StringWriter;

/**
 * @description: 反编译类
 * @author: anyang
 * @create: 2018/10/19 10:01
 */
public class DecompilerProvider {
    private DecompilerSettings settings;
    private DecompilationOptions decompilationOptions;
    private TypeDefinition type;

    private String textContent = "";

    public void generateContent() {
        final StringWriter stringwriter = new StringWriter();
        PlainTextOutput plainTextOutput = new PlainTextOutput(stringwriter) {
            @Override
            public void writeDefinition(String text, Object definition, boolean isLocal) {
                super.writeDefinition(text, definition, isLocal);
            }

            @Override
            public void writeReference(String text, Object reference, boolean isLocal) {
                super.writeReference(text, reference, isLocal);
            }
        };
        plainTextOutput.setUnicodeOutputEnabled(decompilationOptions.getSettings().isUnicodeOutputEnabled());
        settings.getLanguage().decompileType(type, plainTextOutput, decompilationOptions);
        textContent = stringwriter.toString();
    }

    public String getTextContent() {
        return textContent;
    }

    public void setDecompilerReferences(DecompilerSettings settings, DecompilationOptions decompilationOptions) {
        this.settings = settings;
        this.decompilationOptions = decompilationOptions;
    }

    public void setType(TypeDefinition type) {
        this.type = type;
    }
}
