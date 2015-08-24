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

public class Document {
    private Document _createdfrom = null;

    long id = 0;

    Document(long id) {
        this.id = id;
    }

    public Document() throws XapianError {
        id = XapianJNI.document_new();
    }

    public Document(Document doc) throws XapianError {
        // must hold a reference to the Document, or the JVM
        // might garbage-collect it on us!
        _createdfrom = doc;
        id = XapianJNI.document_new(doc.id);
    }

    public void addValue(int index, String value) throws XapianError {
        if (value == null)
            return;
        XapianJNI.document_add_value(id, index, value);
    }

    public String getValue(int index) throws XapianError {
        return XapianJNI.docment_get_value(id, index);
    }

    public void removeValue(int index) throws XapianError {
        XapianJNI.document_remove_value(id, index);
    }

    public void clearValues() throws XapianError {
        XapianJNI.document_clear_values(id);
    }

    public String getData() throws XapianError {
        return XapianJNI.document_get_data(id);
    }

    public void setData(String data) throws XapianError {
        XapianJNI.document_set_data(id, data);
    }

    public void addPosting(String term, int position) throws XapianError {
        if (term == null || term.length() == 0)
            return;
        XapianJNI.document_add_posting(id, fixTerm(term), position);
    }

    public void addTerm(String term) throws XapianError {
        if (term == null || term.length() == 0)
            return;
        XapianJNI.document_add_term(id, fixTerm(term));
    }

    public void removePosting(String term, int position) throws XapianError {
        if (term == null || term.length() == 0)
            return;
        term = fixTerm(term);
        XapianJNI.document_remove_posting(id, term, position);
    }

    public void removeTerm(String term) throws XapianError {
        if (term == null || term.length() == 0)
            return;
        term = fixTerm(term);
        XapianJNI.document_remove_term(id, term);
    }

    public void clearTerms() throws XapianError {
        XapianJNI.document_clear_terms(id);
    }

    public int getTermListCount() throws XapianError {
        return XapianJNI.document_termlist_count(id);
    }

    public int getValuesCount() throws XapianError {
        return XapianJNI.document_values_count(id);
    }

    public String toString() {
        try {
            return XapianJNI.document_get_description(id);
        } catch (XapianError xe) {
            throw new XapianRuntimeError(xe);
        }
    }

    private static final String fixTerm(String term) {
        if (term.length() > 150) {
            System.err.println("BIG_TERM: " + term);
            term = term.substring(0, 150);
        }
        return term;
    }

    protected void finalize() throws Throwable {
        XapianJNI.document_finalize(id);
        super.finalize();
    }
}
