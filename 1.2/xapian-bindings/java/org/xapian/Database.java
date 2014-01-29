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

import java.util.ArrayList;
import java.util.List;

public class Database {
    private Database _createdfrom = null;

    long id = 0;

    private List _children;

    protected Database(long id) {
        this.id = id;
    }

    public Database(Database db) throws XapianError {
        this(XapianJNI.database_new(db.id));

        // we *must* hold a reference to the Database passed into us
        // so the JVM won't garbage-collect it out from under us!
        _createdfrom = db;
    }

    public Database(String path) throws XapianError {
        if (path == null)
            throw new IllegalArgumentException("path cannot be null");
        id = XapianJNI.database_new(path);
    }

    public void addDatabase(Database db) throws XapianError {
        XapianJNI.database_add_database(id, db.id);

        // we must hold references to all databases added
        // so the JVM won't garbage-collect them out from under us
        if (_children == null)
            _children = new ArrayList();
        _children.add(db);
    }

    public void reopen() throws XapianError {
        XapianJNI.database_reopen(id);
    }

    public String toString() {
        try {
            return XapianJNI.database_get_description(id);
        } catch (XapianError xe) {
            throw new XapianRuntimeError(xe);
        }
    }

    public int getDocCount() throws XapianError {
        return XapianJNI.database_get_doccount(id);
    }

    public long getLastDocID() throws XapianError {
        return XapianJNI.database_get_lastdocid(id);
    }

    public double getAverageLength() throws XapianError {
        return XapianJNI.database_get_avlength(id);
    }

    public int getTermFreq(String term) throws XapianError {
        return XapianJNI.database_get_termfreq(id, term);
    }

    public boolean termExists(String term) throws XapianError {
        return XapianJNI.database_term_exists(id, term);
    }

    public int getCollectionFreq(String term) throws XapianError {
        return XapianJNI.database_get_collection_freq(id, term);
    }

    public double getDocLength(long docid) throws XapianError {
        return XapianJNI.database_get_doclength(id, docid);
    }

    public void keep_alive() throws XapianError {
        XapianJNI.database_keep_alive(id);
    }

    public Document getDocument(long docid) throws XapianError {
        return new Document(XapianJNI.database_get_document(id, docid));
    }

    public TermIterator getAllTerms() throws XapianError {
        return new TermIterator(XapianJNI.database_allterms_begin(id), XapianJNI.database_allterms_end(id));
    }

    public TermIterator getTermsForField(String fieldname) throws XapianError {
        return getTermsForField(fieldname, "");
    }

    public TermIterator getTermsForField(String fieldname, String prefix) throws XapianError {
        TermIterator itr = getAllTerms();
        itr.setFilter(fieldname + prefix);
        return itr;
    }

    public PositionIterator getPositionIterator(long dbdocid, String term) throws XapianError {
        return new PositionIterator(XapianJNI.database_positionlist_begin(id, dbdocid, term),
                XapianJNI.database_positionlist_end(id, dbdocid, term));
    }

    /**
     * explicitly close this Database right now
     */
    public void finalize() throws Throwable {
        if (_children != null) _children.clear();
        if (id != 0) {
            XapianJNI.database_finalize(id);
            id = 0;
        }
        super.finalize();
    }
}

