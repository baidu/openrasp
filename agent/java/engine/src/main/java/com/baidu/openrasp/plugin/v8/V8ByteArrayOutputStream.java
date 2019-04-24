package com.baidu.openrasp.plugin.v8;

import java.io.ByteArrayOutputStream;

/**
 * This class implements an output stream in which the data is written into a
 * byte array. The buffer automatically grows as data is written to it. The data
 * can be retrieved using <code>toByteArray()</code> and
 * <code>toString()</code>.
 * <p>
 * Closing a <tt>ByteArrayOutputStream</tt> has no effect. The methods in this
 * class can be called after the stream has been closed without generating an
 * <tt>IOException</tt>.
 *
 * @author Arthur van Hoff
 * @since JDK1.0
 */

public class V8ByteArrayOutputStream extends ByteArrayOutputStream {
    /**
     * Retuen the underlying byte array buffer.
     * 
     * @return the current contents of this output stream, as a byte array.
     * @see java.io.ByteArrayOutputStream#size()
     */
    public synchronized byte getByteArray()[] {
        return buf;
    }
}
