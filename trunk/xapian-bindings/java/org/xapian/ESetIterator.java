/**
 Copyright (c) 2003, Technology Concepts & Design, Inc.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are permitted
 provided that the following conditions are met:

 - Redistributions of source code must retain the above copyright notice, this list of conditions
 and the following disclaimer.

 - Redistributions in binary form must reproduce the above copyright notice, this list of conditions
 and the following disclaimer in the documentation and/or other materials provided with the distribution.

 - Neither the name of Technology Concepts & Design, Inc. nor the names of its contributors may be used to
 endorse or promote products derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.
 **/
package org.xapian;

import org.xapian.errors.XapianError;
import org.xapian.errors.XapianRuntimeError;

import java.util.Iterator;

/**
 *
 */
public class ESetIterator implements Iterator {

    long id;
    private long _end_id;

    ESetIterator(long start, long end) {
        id = start;
        _end_id = end;
    }

    public String getTerm() throws XapianError {
        return XapianJNI.esetiterator_get(id);
    }

    public double getWeight() throws XapianError {
        return XapianJNI.esetiterator_get_weight(id);
    }

    public Object next() {
        try {
            XapianJNI.esetiterator_next(id);
            return this;
        } catch (XapianError xe) {
            throw new XapianRuntimeError(xe);
        }
    }

    public Object prev() {
        try {
            XapianJNI.esetiterator_prev(id);
            return this;
        } catch (XapianError xe) {
            throw new XapianRuntimeError(xe);
        }
    }

    public void remove() {
        throw new UnsupportedOperationException("ESetIterator does not support remove()");
    }

    public boolean hasNext() {
        try {
            return !XapianJNI.esetiterator_equals(id, _end_id);
        } catch (XapianError xe) {
            throw new XapianRuntimeError(xe);
        }
    }

    public String toString() {
        try {
            return XapianJNI.esetiterator_get_description(id);
        } catch (XapianError xe) {
            throw new XapianRuntimeError(xe);
        }
    }

    protected void finalize() throws Throwable {
        XapianJNI.esetiterator_finalize(id);
        XapianJNI.esetiterator_finalize(_end_id);
        super.finalize();
    }
}
