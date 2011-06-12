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

public class MSetIterator implements Iterator {
    private MSet _createdfrom = null;

    long id = 0;
    private int _size = 0;
    private int _pos = 0;

    MSetIterator(MSet set) throws XapianError {
        this(XapianJNI.mset_begin(set.id), XapianJNI.mset_size(set.id));
        // must hold a reference to the MSet or the JVM might
        // garbage-collect it on us!
        _createdfrom = set;
    }

    MSetIterator(long id, int size) {
        this.id = id;
        _size = size;
    }

    public long getDocumentId() throws XapianError {
        return XapianJNI.msetiterator_get_db_docid(id);
    }

    public Document getDocument() throws XapianError {
        return new Document(XapianJNI.msetiterator_get_document(id));
    }

    public int getRank() throws XapianError {
        return XapianJNI.msetiterator_get_rank(id);
    }

    public double getWeight() throws XapianError {
        return XapianJNI.msetiterator_get_rank(id);
    }

    public int getCollapseCount() throws XapianError {
        return XapianJNI.msetiterator_get_collapse_count(id);
    }

    public int getPercent() throws XapianError {
        return XapianJNI.msetiterator_get_percent(id);
    }

    public String toString() {
        try {
            return XapianJNI.msetiterator_get_description(id);
        } catch (XapianError xe) {
            throw new XapianRuntimeError(xe);
        }
    }

    public boolean equals(Object o) {
        try {
            MSetIterator itr = (MSetIterator) o;
            return XapianJNI.msetiterator_equals(id, itr.id);
        } catch (XapianError xe) {
            throw new XapianRuntimeError(xe);
        }
    }

    public boolean hasNext() {
        return _pos != _size;
    }

    public Object next() {
        try {
            if (_pos != 0) {
                XapianJNI.msetiterator_next(id);
            }
            _pos++;

            return this;
        } catch (XapianError xe) {
            throw new XapianRuntimeError(xe);
        }
    }

    public Object prev() {
        try {
            XapianJNI.msetiterator_prev(id);
            _pos--;

            return this;
        } catch (XapianError xe) {
            throw new XapianRuntimeError(xe);
        }
    }

    public void remove() {
        // hmm, if we were smart, we could support this
        // all we need is the database_id from which we were created
        throw new UnsupportedOperationException("MSetIterator does not support remove");
    }

    protected void finalize() throws Throwable {
        XapianJNI.msetiterator_finalize(id);
        super.finalize();
    }
}
