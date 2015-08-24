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


import org.xapian.errors.*;

public class WritableDatabase extends Database {

    protected WritableDatabase(long id) {
        super(id);
    }

    public WritableDatabase() throws XapianError {
        super(XapianJNI.writabledatabase_new());
    }

    public WritableDatabase(String path, int mode) throws XapianError {
        super(XapianJNI.writabledatabase_new(path, mode));
    }

    public void flush() throws XapianError, DatabaseError {
        XapianJNI.writabledatabase_flush(id);
    }

    public void beginTransaction() throws XapianError, UnimplementedError, InvalidOperationError {
        XapianJNI.writabledatabase_begin_transaction(id);
    }

    public void commitTransaction() throws XapianError, DatabaseError, UnimplementedError, InvalidOperationError {
        XapianJNI.writabledatabase_commit_transaction(id);
    }

    public void cancelTransaction() throws XapianError, DatabaseError, UnimplementedError, InvalidOperationError {
        XapianJNI.writabledatabase_cancel_transaction(id);
    }

    public long addDocument(Document doc) throws XapianError, DatabaseError {
        return XapianJNI.writabledatabase_add_document(id, doc.id);
    }

    public void deleteDocument(long docid) throws XapianError, DatabaseError {
        XapianJNI.writabledatabase_delete_document(id, docid);
    }

    public void replaceDocument(long which_docid, Document newdoc) throws XapianError, DatabaseError {
        XapianJNI.writabledatabase_replace_document(id, which_docid, newdoc.id);
    }

    public String toString() {
        try {
            return XapianJNI.writabledatabase_get_description(id);
        } catch (XapianError xe) {
            throw new XapianRuntimeError(xe);
        }
    }

    /**
     * explicitly close this WritableDatabase right now
     */
    public void finalize() throws Throwable {
        if (id != 0) {
            XapianJNI.writabledatabase_finalize(id);
            id = 0;
        }

        super.finalize();
    }
}
