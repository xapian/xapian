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

import org.xapian.errors.InvalidOperationError;
import org.xapian.errors.XapianError;
import org.xapian.errors.XapianRuntimeError;

import java.util.Iterator;

public class TermIterator implements Iterator {

    private class Info {
        String term;
        int freq;
        long wdf;

        void set(String term, int freq, long wdf) {
            this.term = term;
            this.freq = freq;
            this.wdf = wdf;
        }
    }

    long id;
    private long _end;
    private final long _dbdocid;
    private Info _info = new Info();
    private String _filter = null;
    private boolean _hasNext = true;

    TermIterator(long id, long end_id) throws XapianError {
        this(0, id, end_id);
    }

    TermIterator(long dbdocid, long id, long end_id) throws XapianError {
        _dbdocid = dbdocid;
        this.id = id;
        _end = end_id;
        _hasNext = !XapianJNI.termiterator_equals(id, end_id);
    }

    public void skip_to(String term) throws XapianError {
        XapianJNI.termiterator_skip_to(id, term);
    }

    public void setFilter(String filter) throws XapianError {
        _filter = filter;
        skip_to(filter);
        if (XapianJNI.termiterator_equals(id, _end))
            _hasNext = false;
        else if (_filter != null && !XapianJNI.termiterator_get_termname(id).startsWith(_filter))
            _hasNext = false;
        else
            _hasNext = true;
    }

    public boolean hasNext() {
        return _hasNext;
    }

    public Object next() {
        try {
            String termname = XapianJNI.termiterator_get_termname(id);
            _info.set(termname,
                    _dbdocid != 0 ? 0 : XapianJNI.termiterator_get_term_freq(id), // can't get term frequency when we have a document id
                    XapianJNI.termiterator_get_wdf(id));

            XapianJNI.termiterator_next(id);
            if (XapianJNI.termiterator_equals(id, _end))
                _hasNext = false;
            else if (_filter != null && !XapianJNI.termiterator_get_termname(id).startsWith(_filter))
                _hasNext = false;
            else
                _hasNext = true;
            return termname;
        } catch (XapianError xe) {
            throw new XapianRuntimeError(xe);
        }
    }

    public int getTermFrequency() {
        return _info.freq;
    }

    public long getWdf() {
        return _info.wdf;
    }

    public PositionIterator getPositionListIterator(Database db) throws XapianError {
        if (_dbdocid == 0)
            throw new InvalidOperationError("No Document id associated with this TermIterator");

        return db.getPositionIterator(_dbdocid, _info.term);

        // this is what I think should happen, but Xapian throws:
        //      InvalidOperationError: positionlist_begin not supported
        // return new PositionIterator (XapianJNI.termiterator_positionlist_begin(id), XapianJNI.termiterator_positionlist_end(id));
    }


    public void remove() {
        throw new UnsupportedOperationException("TermIterator.remove() not supported");
    }

    protected void finalize() throws Throwable {
        XapianJNI.termiterator_finalize(id);
        XapianJNI.termiterator_finalize(_end);
        super.finalize();
    }

    public String toString() {
        try {
            return XapianJNI.termiterator_get_description(id);
        } catch (XapianError xe) {
            throw new XapianRuntimeError(xe);
        }
    }
}
