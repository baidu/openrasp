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

package com.baidu.openrasp.messaging;

import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;
import java.nio.charset.Charset;
import java.nio.charset.CharsetEncoder;

/**
 * Created by lxk on 11/29/17.
 */
public class SyslogTcpWriter extends OutputStreamWriter {

    int syslogFacility;
    int level;

    public SyslogTcpWriter(OutputStream out, String charsetName, int syslogFacility) throws UnsupportedEncodingException {
        super(out, charsetName);
        this.syslogFacility = syslogFacility;
    }

    public SyslogTcpWriter(OutputStream out, int syslogFacility) {
        super(out);
        this.syslogFacility = syslogFacility;
    }

    public SyslogTcpWriter(OutputStream out, Charset cs, int syslogFacility) {
        super(out, cs);
        this.syslogFacility = syslogFacility;
    }

    public SyslogTcpWriter(OutputStream out, CharsetEncoder enc, int syslogFacility) {
        super(out, enc);
        this.syslogFacility = syslogFacility;
    }

    public
    void setLevel(int level) {
        this.level = level;
    }

    public
    void setSyslogFacility(int syslogFacility) {
        this.syslogFacility = syslogFacility;
    }

    public
    void writeString(String string) throws IOException{
        write("<"+(syslogFacility | level)+">" + string);
    }
}
