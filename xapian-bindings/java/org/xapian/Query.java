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
import org.xapian.query_parser.ParseException;
import org.xapian.query_parser.QueryParser;

import java.io.StringReader;

public class Query {
    public static final int OP_AND = 1;
    public static final int OP_OR = 2;
    public static final int OP_AND_NOT = 3;
    public static final int OP_XOR = 4;
    public static final int OP_AND_MAYBE = 5;
    public static final int OP_FILTER = 6;
    public static final int OP_NEAR = 7;
    public static final int OP_PHRASE = 8;
    public static final int OP_WEIGHT_CUTOFF = 9;
    public static final int OP_ELITE_SET = 10;

    private Query _left = null, _right = null;
    long id = -1;

    private String _nativeStuff;

    public static Query parse(String query) throws ParseException, XapianError {
        QueryParser parser = new QueryParser(new StringReader(query));
        Query q = parser.parse();
        if (q == null)
            throw new ParseException("syntax error: " + query);
        q.setNativeStuff(parser.getNativeStuff());
        return q;
    }

    private static final void validateOperator(int op) throws IllegalArgumentException {
        if (op < 1 || op > 10)
            throw new IllegalArgumentException("operator " + op + " is not valid");
    }

    public Query() throws XapianError {
        id = XapianJNI.query_new();
    }

    public Query(String term) throws XapianError {
        id = XapianJNI.query_new(term);
    }

    public Query(int operator, Query left, Query right) throws XapianError {
        validateOperator(operator);
        _left = left;
        _right = right;

        if (right == null)
            id = XapianJNI.query_new(operator, left.id);
        else if (left == null)
            id = XapianJNI.query_new(operator, right.id);
        else
            id = XapianJNI.query_new(operator, left.id, right.id);
    }

    public Query(int operator, String left, String right) throws XapianError {
        validateOperator(operator);
        id = XapianJNI.query_new(operator, left, right);
    }

    public Query(int operator, String[] terms) throws XapianError {
        validateOperator(operator);
        id = XapianJNI.query_new(operator, terms);
    }

    public Query(int operator, Query q) throws XapianError {
        validateOperator(operator);
        _right = q;
        id = XapianJNI.query_new(operator, q.id);
    }

    public String getNativeStuff() {
        return _nativeStuff;
    }

    void setNativeStuff(String str) {
        _nativeStuff = str;
    }

    public void setWindow(long termpos) throws XapianError {
        XapianJNI.query_set_window(id, termpos);
    }

    public void setCutoff(double cutoff) throws XapianError {
        XapianJNI.query_set_cutoff(id, cutoff);
    }

    public void setEliteSetSize(long size) throws XapianError {
        XapianJNI.query_set_elite_set_size(id, size);
    }

    public long getLength() throws XapianError {
        return XapianJNI.query_get_length(id);
    }

    public long setLength(long qlen) throws XapianError {
        return XapianJNI.query_set_length(id, qlen);
    }

    public TermIterator getTerms() throws XapianError {
        return new TermIterator(id, XapianJNI.query_terms_begin(id), XapianJNI.query_terms_end(id));
    }

    public boolean isEmpty() throws XapianError {
        return XapianJNI.query_is_empty(id);
    }

    public String toString() {
        try {
            return XapianJNI.query_get_description(id);
        } catch (XapianError xe) {
            return xe.toString();
        }
    }

    protected void finalize() throws Throwable {
        XapianJNI.query_finalize(id);
        super.finalize();
    }

}
