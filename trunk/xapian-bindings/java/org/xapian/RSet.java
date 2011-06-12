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

/**
 *
 */
public class RSet {
    long id = 0;

    RSet(long id) {
        this.id = id;
    }

    public RSet() throws XapianError {
        id = XapianJNI.rset_new();
    }

    public long size() throws XapianError {
        return XapianJNI.rset_size(id);
    }

    public boolean empty() throws XapianError {
        return XapianJNI.rset_empty(id);
    }

    public void addDocument(long dbdocid) throws XapianError {
        XapianJNI.rset_add_document(id, dbdocid);
    }

    public void addDocument(MSetIterator itr) throws XapianError {
        XapianJNI.rset_add_document_via_msetiterator(id, itr.id);
    }

    public void removeDocument(long dbdocid) throws XapianError {
        XapianJNI.rset_remove_document(id, dbdocid);
    }

    public void removeDocument(MSetIterator itr) throws XapianError {
        XapianJNI.rset_remove_document_via_msetiterator(id, itr.id);
    }

    public boolean contains(long dbdocid) throws XapianError {
        return XapianJNI.rset_contains(id, dbdocid);
    }

    public boolean contains(MSetIterator itr) throws XapianError {
        return XapianJNI.rset_contains_via_msetiterator(id, itr.id);
    }

    public String toString() {
        try {
            return XapianJNI.rset_get_description(id);
        } catch (XapianError xe) {
            throw new XapianRuntimeError(xe);
        }
    }

    protected void finalize() throws Throwable {
        XapianJNI.rset_finalize(id);
        super.finalize();
    }
}
