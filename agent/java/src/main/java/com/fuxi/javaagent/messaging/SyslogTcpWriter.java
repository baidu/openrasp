/**
 * Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

package com.fuxi.javaagent.messaging;

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
