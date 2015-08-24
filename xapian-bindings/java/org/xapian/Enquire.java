/**
 Copyright (c) 2003, Technology Concepts & Design, Inc.
 Copyright (c) 2006,2008, Olly Betts
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
public class Enquire {
    private Database _database = null;
    private Query _query = null;

    long id = 0;

    public Enquire(Database db) throws XapianError {
        // must hold a reference to the database, otherwise the JVM
        // might garbage-collect it on us!
        _database = db;
        id = XapianJNI.enquire_new(db.id);
    }

    public void setQuery(Query q) throws XapianError {
	setQuery(q, 0);
    }

    public void setQuery(Query q, int qlen) throws XapianError {
        _query = q;
        XapianJNI.enquire_set_query(id, q.id, qlen);
    }

    public Query getQuery() throws XapianError {
        // No need to call the JNI method here.
        return _query;
    }

    //
    // TODO: Implement Weight so I cna implement htis method
    //

    /**
     * public void setWeightingScheme (Weight weight) {
     * }
     */

    public void setCollapseKey(long collapse_key) throws XapianError {
        XapianJNI.enquire_set_collapse_key(id, collapse_key);
    }

    public void setSortForward(boolean forward) throws XapianError {
        XapianJNI.enquire_set_sort_forward(id, forward);
    }

    public void setCutoff(int percent_cutoff) throws XapianError {
        setCutoff(percent_cutoff, 0);
    }

    public void setCutoff(int percent_cutoff, double weight_cutoff) throws XapianError {
        XapianJNI.enquire_set_cutoff(id, percent_cutoff, weight_cutoff);
    }

    public void setSorting(long sort_key, int sort_bands) throws XapianError {
        XapianJNI.enquire_set_sorting(id, sort_key, sort_bands);
    }

    public MSet getMSet(long first, long maxitems) throws XapianError {
        return getMSet(first, maxitems, null, null);
    }

    public MSet getMSet(long first, long maxitems, RSet rset, MatchDecider md) throws XapianError {
        return new MSet(XapianJNI.enquire_get_mset(id, first, maxitems, rset == null ? 0 : rset.id, md));
    }

    public ESet getESet(long maxitems, RSet rset, int flags, double k, ExpandDecider ed) throws XapianError {
        return new ESet(XapianJNI.enquire_get_eset(id, maxitems, rset.id, flags, k, ed));
    }

    public ESet getESet(long maxitems, RSet rset) throws XapianError {
        return new ESet(XapianJNI.enquire_get_eset(id, maxitems, rset.id, null));
    }

    public ESet getESet(long maxitems, RSet rset, ExpandDecider ed) throws XapianError {
        return new ESet(XapianJNI.enquire_get_eset(id, maxitems, rset.id, ed));
    }

    public TermIterator getMatchingTerms(long dbdocid) throws XapianError {
        return new TermIterator(dbdocid, XapianJNI.enquire_get_matching_terms_begin(id, dbdocid), XapianJNI.enquire_get_matching_terms_end(id, dbdocid));
    }

    public TermIterator getMatchingTerms(MSetIterator itr) throws XapianError {
        return new TermIterator(itr.getDocumentId(), XapianJNI.enquire_get_matching_terms_begin_by_msetiterator(id, itr.id), XapianJNI.enquire_get_matching_terms_end_by_msetiterator(id, itr.id));
    }

    public String toString() {
        try {
            return XapianJNI.enquire_get_description(id);
        } catch (XapianError xe) {
            throw new XapianRuntimeError(xe);
        }
    }

    protected void finalize() throws Throwable {
        XapianJNI.enquire_finalize(id);
        super.finalize();
    }

}
