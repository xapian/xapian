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
//import org.xapian.query_parser.ParseException;
//import org.xapian.query_parser.QueryParser;

//import java.io.StringReader;

public class Query {
    public static final int OP_AND = 1;
    public static final int OP_OR = 2;
    public static final int OP_AND_NOT = 3;
    public static final int OP_XOR = 4;
    public static final int OP_AND_MAYBE = 5;
    public static final int OP_FILTER = 6;
    public static final int OP_NEAR = 7;
    public static final int OP_PHRASE = 8;
    public static final int OP_ELITE_SET = 10;

    private Query _left = null, _right = null;
    long id = 0;
/*
    private String _nativeStuff;
    private String _nativeOperator;

    public static Query parse(String query) throws ParseException, XapianError {
        if (query == null || query.trim().length() == 0)
            throw new ParseException("Empty Queries are not allowed");

        QueryParser parser = new QueryParser(new StringReader(query));
        Query q = parser.parse();
        if (q == null)
            throw new ParseException("syntax error: " + query);
        q.setNativeStuff(parser.getNativeStuff());
        q.setNativeOperator(parser.getNativeOperator());
        return q;
    }
*/
    private static final void validateOperator(int op) throws IllegalArgumentException {
        if (op < 1 || op > 10 || op == 9)
            throw new IllegalArgumentException("operator " + op + " is not valid");
    }

    public Query() throws XapianError {
        id = XapianJNI.query_new();
    }

    public Query(String term) throws XapianError {
        if (term == null)
            throw new XapianError("Empty Queries are not allowed");
        id = XapianJNI.query_new(term);
    }

    public Query(String term, int wqf) throws XapianError {
        if (term == null)
            throw new XapianError("Empty Queries are not allowed");
        id = XapianJNI.query_new(term, wqf);
    }

    public Query(String term, int wqf, int pos) throws XapianError {
        if (term == null)
            throw new XapianError("Empty Queries are not allowed");
        id = XapianJNI.query_new(term, wqf, pos);
    }

    public Query(int operator, Query left, Query right) throws XapianError {
        validateOperator(operator);

        // must hold onto left and right Queries or the JVM might
        // garbage-collect them on us!
        _left = left;
        _right = right;

        if (left == null)
	    throw new XapianError("Left side of query is null");
        if (right == null)
	    throw new XapianError("Right side of query is null");
	id = XapianJNI.query_new(operator, left.id, right.id);
    }

    public Query(int operator, String left, String right) throws XapianError {
        validateOperator(operator);
        id = XapianJNI.query_new(operator, left, right);
    }

    public Query(int operator, Query[] queries) throws XapianError {
        validateOperator(operator);
	long[] query_ids = new long[queries.length];
	for (int i = 0; i < queries.length; ++i) {
	    query_ids[i] = queries[i].id;
	}
	id = XapianJNI.query_new(operator, query_ids);
    }

    public Query(int operator, String[] terms) throws XapianError {
        validateOperator(operator);
        id = XapianJNI.query_new(operator, terms);
    }

/*
    public String getNativeStuff() {
        return _nativeStuff;
    }

    void setNativeStuff(String str) {
        _nativeStuff = str;
    }

    public String getNativeOperator() {
        return _nativeOperator;
    }

    void setNativeOperator(String str) {
        _nativeOperator = str;
    }
*/

    public long getLength() throws XapianError {
        return XapianJNI.query_get_length(id);
    }

    public TermIterator getTerms() throws XapianError {
        return new TermIterator(id, XapianJNI.query_terms_begin(id), XapianJNI.query_terms_end(id));
    }

    public boolean empty() throws XapianError {
        return XapianJNI.query_empty(id);
    }

    public boolean isEmpty() throws XapianError {
        return XapianJNI.query_empty(id);
    }

    public String toString() {
        try {
            return XapianJNI.query_get_description(id);
        } catch (XapianError xe) {
            throw new XapianRuntimeError(xe);
        }
    }

    protected void finalize() throws Throwable {
        XapianJNI.query_finalize(id);
        super.finalize();
    }

}
