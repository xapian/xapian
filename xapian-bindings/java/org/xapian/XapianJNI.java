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

import org.xapian.errors.*;


/**
 * Static proxy class that gets us access to Xapian.<p>
 * <p/>
 * Although this class is defined as <i>public</i>, it is not
 * meant to be used outside this package.  It is available
 * for those that desire lower-level access to Xapian without
 * going through the API classes in this package.
 */
public class XapianJNI {

    static {
        // static initialization for loading the native library
        System.loadLibrary("xapian_jni");
    }


    // functions to support the Xapian::Auto namespace
    public static final int DB_CREATE_OR_OPEN = 1;
    public static final int DB_CREATE = 2;
    public static final int DB_CREATE_OR_OVERWRITE = 3;
    public static final int DB_OPEN = 4;

    /* Returns id of a Xapian::Database (deprecated) */
    public static native long auto_open(String path) throws XapianError;

    /* Returns id of a Xapian::WritableDatabase (deprecated) */
    public static native long auto_open(String path, int mode) throws XapianError;

    /* Returns id of a Xapian::Database */
    public static native long auto_open_stub(String path) throws XapianError;

    // support for the Xapian::InMemory namespace
    /* Returns id of a Xapian::WritableDatabase */
    public static native long inmemory_open() throws XapianError;

    // support for the Xapian::Quartz namespace
    /* Returns id of a Xapian::Database */
    public static native long quartz_open(String dir) throws XapianError;

    /* Returns id of a Xapian::WritableDatabase */
    public static long quartz_open(String dir, int action) throws XapianError {
        return quartz_open(dir, action, 8192);
    }

    /* Returns id of a Xapian::WritableDatabase */
    public static native long quartz_open(String dir, int action, int block_size) throws XapianError;

    // support for the Xapian::Remote namespace
    /* Returns id of a Xapian::Database */
    public static long remote_open(String program, String args) throws XapianError {
        return remote_open(program, args, 10000);
    }

    /* Returns id of a Xapian::Database */
    public static native long remote_open(String program, String args, int timeout) throws XapianError;

    /* Returns id of a Xapian::Database */
    public static long remote_open(String host, int port) throws XapianError {
        return remote_open(host, port, 10000, 0);
    }

    public static native long remote_open(String host, int port, int timeout, int connect_timeout) throws XapianError;


    // database functions
    public static native long database_new() throws XapianError;

    public static native long database_new(String path) throws XapianError;

    public static native long database_new(long dbid) throws XapianError;

    public static native void database_add_database(long dbid, long toadd_id) throws XapianError;

    public static native void database_reopen(long dbid) throws XapianError;

    public static native String database_get_description(long dbid) throws XapianError;

    public static native long database_postlist_begin(long dbid, String term) throws XapianError;

    public static native long database_postlist_end(long dbid, String term) throws XapianError;

    public static native long database_termlist_begin(long dbid, long assigned_docid) throws XapianError;

    public static native long database_termlist_end(long dbid, long assigned_docid) throws XapianError;

    public static native long database_positionlist_begin(long dbid, long dbdocid, String term) throws XapianError;

    public static native long database_positionlist_end(long dbid, long dbdocid, String term) throws XapianError;

    public static native long database_allterms_begin(long dbid) throws XapianError;

    public static native long database_allterms_end(long dbid) throws XapianError;

    public static native int database_get_doccount(long dbid) throws XapianError;

    public static native long database_get_lastdocid(long dbid) throws XapianError;

    public static native double database_get_avlength(long dbid) throws XapianError;

    public static native int database_get_termfreq(long dbid, String term) throws XapianError;

    public static native boolean database_term_exists(long dbid, String term) throws XapianError;

    public static native int database_get_collection_freq(long dbid, String term) throws XapianError;

    public static native double database_get_doclength(long dbid, long assigned_docid) throws XapianError;

    public static native void database_keep_alive(long dbid) throws XapianError;

    public static native long database_get_document(long dbid, long assigned_docid) throws XapianError, DocNotFoundError;

    public static native void database_finalize(long dbid);

    // writable-database functions
    public static native long writabledatabase_new() throws XapianError;

    public static native long writabledatabase_new(String path, int mode) throws XapianError;

    public static native void writabledatabase_flush(long dbid) throws DatabaseError, DatabaseCorruptError, DatabaseLockError, XapianError;

    public static native void writabledatabase_begin_transaction(long dbid) throws UnimplementedError, InvalidOperationError, XapianError;

    public static native void writabledatabase_commit_transaction(long dbid) throws DatabaseError, DatabaseCorruptError, InvalidOperationError, UnimplementedError, XapianError;

    public static native void writabledatabase_cancel_transaction(long dbid) throws DatabaseError, DatabaseCorruptError, InvalidOperationError, UnimplementedError, XapianError;

    public static native long writabledatabase_add_document(long dbid, long docid) throws DatabaseError, DatabaseCorruptError, DatabaseLockError, XapianError;

    public static native void writabledatabase_delete_document(long dbid, long assigned_docid) throws DatabaseError, DatabaseCorruptError, DatabaseLockError, XapianError;

    public static native void writabledatabase_replace_document(long dbid, long assigned_docid, long docid) throws DatabaseError, DatabaseCorruptError, DatabaseLockError, XapianError;

    public static native String writabledatabase_get_description(long dbid) throws XapianError;

    public static native void writabledatabase_finalize(long dbid);


    // document functions
    public static native long document_new() throws XapianError;

    public static native long document_new(long docid) throws XapianError;

    public static native String docment_get_value(long docid, int value_index) throws XapianError;

    public static native void document_add_value(long docid, int value_index, String value) throws XapianError, InvalidArgumentError;

    public static native void document_remove_value(long docid, int value_index) throws XapianError, InvalidArgumentError;

    public static native void document_clear_values(long docid) throws XapianError;

    public static native String document_get_data(long docid) throws XapianError;

    public static native void document_set_data(long docid, String data) throws XapianError, InvalidArgumentError;

    public static native void document_add_posting(long docid, String term, int position) throws XapianError, InvalidArgumentError;

    public static native void document_add_term(long docid, String term) throws XapianError, InvalidArgumentError;

    /*
     * For compatibility with older code.
     * @deprecated use document_add_term instead
     */
    public static void document_add_term_nopos(long docid, String term) throws XapianError, InvalidArgumentError {
        document_add_term(docid, term);
    }

    public static native void document_remove_posting(long docid, String term, int position) throws XapianError, InvalidArgumentError;

    public static native void document_remove_term(long docid, String term) throws XapianError, InvalidArgumentError;

    public static native void document_clear_terms(long docid) throws XapianError;

    public static native int document_termlist_count(long docid) throws XapianError;

    public static native int document_values_count(long docid) throws XapianError;

    public static native String document_get_description(long docid) throws XapianError;

    public static native void document_finalize(long docid);


    // mset functions
    public static native int mset_convert_to_percent(long msetid, long other_msetid) throws XapianError;

    public static native int mset_convert_to_percent(long msetid, double weight) throws XapianError;

    public static native int mset_get_termfreq(long msetid, String term) throws XapianError, InvalidArgumentError;

    public static native double mset_get_termweight(long msetid, String term) throws XapianError, InvalidArgumentError;

    public static native int mset_get_firstitem(long msetid) throws XapianError;

    public static native int mset_get_matches_lower_bound(long msetid) throws XapianError;

    public static native int mset_get_matches_estimated(long msetid) throws XapianError;

    public static native int mset_get_matches_upper_bound(long msetid) throws XapianError;

    public static native double mset_get_max_possible(long msetid) throws XapianError;

    public static native double mset_get_max_attained(long msetid) throws XapianError;

    public static native int mset_size(long msetid) throws XapianError;

    public static native boolean mset_empty(long msetid) throws XapianError;

    public static native long mset_begin(long msetid) throws XapianError;

    public static native long mset_end(long msetid) throws XapianError;

    public static native long mset_back(long msetid) throws XapianError;

    public static native long mset_element(long msetid, long index) throws XapianError;

    public static native String mset_get_description(long msetid) throws XapianError;

    public static native void mset_finalize(long msetid);

    // mset iterator functions
    public static native long msetiterator_get_document(long msetiteratorid) throws XapianError, DocNotFoundError;

    public static native int msetiterator_get_rank(long msetiteratorid) throws XapianError;

    public static native double msetiterator_get_weight(long msetiteratorid) throws XapianError;

    public static native int msetiterator_get_collapse_count(long msetiteratorid) throws XapianError;

    public static native int msetiterator_get_percent(long msetiteratorid) throws XapianError;

    public static native String msetiterator_get_description(long msetiteratorid) throws XapianError;

    public static native void msetiterator_prev(long msetiteratorid) throws XapianError;

    public static native void msetiterator_next(long msetiteratorid) throws XapianError;

    public static native long msetiterator_get_db_docid(long msetiteratorid) throws XapianError;

    public static native boolean msetiterator_equals(long a, long b) throws XapianError;

    public static native void msetiterator_finalize(long msetiteratorid);


    // term iterator functions
    public static native void termiterator_next(long termiteratorid) throws XapianError;

    public static native String termiterator_get_termname(long termiteratorid) throws XapianError;

    public static native int termiterator_get_term_freq(long termiteratorid) throws XapianError;

    public static native long termiterator_get_wdf(long termiteratorid) throws XapianError;

    public static native void termiterator_skip_to(long termiteratorid, String term) throws XapianError;

    public static native String termiterator_get_description(long termiteratorid) throws XapianError;

    public static native long termiterator_positionlist_begin(long termiteratorid) throws XapianError;

    public static native long termiterator_positionlist_end(long termiteratorid) throws XapianError;

    public static native boolean termiterator_equals(long a, long b) throws XapianError;

    public static native void termiterator_finalize(long termiteratorid);

    // query functions
    public static native long query_new() throws XapianError;

    public static native long query_new(String term) throws XapianError;

    public static native long query_new(String term, int wqf) throws XapianError;

    public static native long query_new(String term, int wqf, int pos) throws XapianError;

    public static native long query_new(int operator, long left, long right) throws XapianError;

    public static native long query_new(int operator, String left, String right) throws XapianError;

    public static native long query_new(int operator, String[] terms) throws XapianError;

    public static native long query_new(int operator, long[] queries) throws XapianError;

    public static native long query_new(int operator, long id) throws XapianError;

    public static native String query_get_description(long id) throws XapianError;

    public static native boolean query_empty(long id) throws XapianError;

    public static native long query_terms_begin(long id) throws XapianError;

    public static native long query_terms_end(long id) throws XapianError;

    public static native long query_get_length(long id) throws XapianError;

    public static native void query_finalize(long id);

    // enquire functions
    public static native long enquire_new(long dbid) throws XapianError;

    public static native void enquire_set_query(long eid, long queryid) throws XapianError;

    public static native void enquire_set_query(long eid, long queryid, int qlen) throws XapianError;

// We implement get_query in Java and don't use JNI or C++ at all for it.
//    public static native long enquire_get_query(long eid) throws XapianError;
//    public static native void enquire_set_weighting_scheme(long eid, long weightid) throws XapianError;
    public static native void enquire_set_collapse_key(long eid, long collapse_key) throws XapianError;

    public static native void enquire_set_sort_forward(long eid, boolean forward) throws XapianError;

    public static native void enquire_set_cutoff(long eid, int percent_cutoff, double weight_cutoff) throws XapianError;

    public static native void enquire_set_sorting(long eid, long sort_key, int sort_bands) throws XapianError;

    public static native long enquire_get_mset(long eid, long first, long maxitems, long rsetid, MatchDecider md) throws XapianError;

    public static native long enquire_get_eset(long eid, long maxitems, long rsetid, int flags, double k, ExpandDecider ed) throws XapianError;

    public static native long enquire_get_eset(long eid, long maxitems, long rsetid, ExpandDecider ed) throws XapianError;

    public static native long enquire_get_matching_terms_begin(long eid, long dbdocid) throws XapianError;

    public static native long enquire_get_matching_terms_end(long eid, long dbdocid) throws XapianError;

    public static native long enquire_get_matching_terms_begin_by_msetiterator(long eid, long msetiteratorid) throws XapianError;

    public static native long enquire_get_matching_terms_end_by_msetiterator(long eid, long msetiteratorid) throws XapianError;

    public static native String enquire_get_description(long eid) throws XapianError;

    public static native void enquire_finalize(long eid);

    // rset functions
    public static native long rset_new() throws XapianError;

    public static native long rset_size(long rsetid) throws XapianError;

    public static native boolean rset_empty(long rsetid) throws XapianError;

    public static native void rset_add_document(long rsetid, long dbdocid) throws XapianError;

    public static native void rset_add_document_via_msetiterator(long rsetid, long msetiteratorid) throws XapianError;

    public static native void rset_remove_document(long rsetid, long dbdocid) throws XapianError;

    public static native void rset_remove_document_via_msetiterator(long rsetid, long msetiteratorid) throws XapianError;

    public static native boolean rset_contains(long rsetid, long dbdocid) throws XapianError;

    public static native boolean rset_contains_via_msetiterator(long rsetid, long msetiteratorid) throws XapianError;

    public static native String rset_get_description(long rsetid) throws XapianError;

    public static native void rset_finalize(long rsetid);

    // eset functions
    public static native long eset_new() throws XapianError;

    public static native long eset_get_ebound(long esetid) throws XapianError;

    public static native long eset_size(long esetid) throws XapianError;

    public static native boolean eset_empty(long esetid) throws XapianError;

    public static native long eset_begin(long esetid) throws XapianError;

    public static native long eset_end(long esetid) throws XapianError;

    public static native String eset_get_description(long esetid) throws XapianError;

    public static native void eset_finalize(long esetid);

    // esetiterator functions
    public static native long esetiterator_new() throws XapianError;

    public static native void esetiterator_prev(long esetiteratorid) throws XapianError;

    public static native void esetiterator_next(long esetiteratorid) throws XapianError;

    public static native boolean esetiterator_equals(long a, long b) throws XapianError;

    public static native String esetiterator_get(long esetiteratorid) throws XapianError;

    public static native double esetiterator_get_weight(long esetiteratorid) throws XapianError;

    public static native String esetiterator_get_description(long esetiteratorid) throws XapianError;

    public static native void esetiterator_finalize(long esetiteratorid);

    // stem functions
    public static native long stem_new() throws XapianError;

    public static native long stem_new(String language) throws XapianError;

    public static native String stem_stem_word(long stemid, String word) throws XapianError;

    public static native String stem_get_description(long stemid) throws XapianError;

    public static native String stem_get_available_languages() throws XapianError;

    public static native void stem_finalize(long stemid);

    // position iterator functions
    public static native long positioniterator_getvalue(long id) throws XapianError;

    public static native void positioniterator_next(long id) throws XapianError;

    public static native boolean positioniterator_equals(long a, long b) throws XapianError;

    public static native String positioniterator_get_description(long id) throws XapianError;

    public static native void positioniterator_finalize(long id);
}
