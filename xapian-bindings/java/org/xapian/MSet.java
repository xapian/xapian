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


public class MSet {

    long id = 0;

    MSet(long id) {
        this.id = id;
    }

    public int convertToPercent(MSet mset) throws XapianError {
        return XapianJNI.mset_convert_to_percent(id, mset.id);
    }

    public int converToPercent(double weight) throws XapianError {
        return XapianJNI.mset_convert_to_percent(id, weight);
    }

    public int getTermFrequency(String term) throws XapianError {
        return XapianJNI.mset_get_termfreq(id, term);
    }

    public double getTermWeight(String term) throws XapianError {
        return XapianJNI.mset_get_termweight(id, term);
    }

    public int getFirstItem() throws XapianError {
        return XapianJNI.mset_get_firstitem(id);
    }

    public int getMatchesLowerBound() throws XapianError {
        return XapianJNI.mset_get_matches_lower_bound(id);
    }

    public int getMatchesEstimated() throws XapianError {
        return XapianJNI.mset_get_matches_estimated(id);
    }

    public int getMatchesUpperBound() throws XapianError {
        return XapianJNI.mset_get_matches_upper_bound(id);
    }

    public double getMaxPossible() throws XapianError {
        return XapianJNI.mset_get_max_possible(id);
    }

    public double getMaxAttained() throws XapianError {
        return XapianJNI.mset_get_max_attained(id);
    }

    public int size() throws XapianError {
        return XapianJNI.mset_size(id);
    }

    public MSetIterator getElement(long index) throws XapianError {
        return new MSetIterator(XapianJNI.mset_element(id, index), 1);
    }

    public MSetIterator iterator() throws XapianError {
        return new MSetIterator(this);
    }

    public String toString() {
        try {
            return XapianJNI.mset_get_description(id);
        } catch (XapianError xe) {
            throw new XapianRuntimeError(xe);
        }
    }

    protected void finalize() throws Throwable {
        XapianJNI.mset_finalize(id);
        super.finalize();
    }
}
