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
 
#include "xapian_jni.h"

using namespace std;
using namespace Xapian;

static Xapian::Query::op op_table[] = {
        Query::OP_AND,
        Query::OP_OR,
        Query::OP_AND_NOT,
        Query::OP_XOR,
        Query::OP_AND_MAYBE,
        Query::OP_FILTER,
        Query::OP_NEAR,
        Query::OP_PHRASE,
        Query::OP_ELITE_SET,
};


JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_query_1new__ (JNIEnv *env, jclass clazz) {
    TRY
        Query *q = new Query();
        return _query->put(q);
    CATCH(-1)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_query_1new__Ljava_lang_String_2 (JNIEnv *env, jclass clazz, jstring term) {
    TRY
        const char *c_term = env->GetStringUTFChars(term, 0);
        Query *q = new Query(string(c_term, env->GetStringUTFLength(term)));
        env->ReleaseStringUTFChars(term, c_term);
        return _query->put(q);
    CATCH(-1)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_query_1new__Ljava_lang_String_2I (JNIEnv *env, jclass clazz, jstring term, jint wqf) {
    TRY
        const char *c_term = env->GetStringUTFChars(term, 0);
        Query *q;
	q = new Query(string(c_term, env->GetStringUTFLength(term)), wqf);
        env->ReleaseStringUTFChars(term, c_term);
        return _query->put(q);
    CATCH(-1)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_query_1new__Ljava_lang_String_2II (JNIEnv *env, jclass clazz, jstring term, jint wqf, jint pos) {
    TRY
        const char *c_term = env->GetStringUTFChars(term, 0);
        Query *q;
	q = new Query(string(c_term, env->GetStringUTFLength(term)), wqf, pos);
        env->ReleaseStringUTFChars(term, c_term);
        return _query->put(q);
    CATCH(-1)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_query_1new__IJJ (JNIEnv *env, jclass clazz, jint op, jlong leftid, jlong rightid) {
    TRY
        Query *left = _query->get(leftid);
        Query *right = _query->get(rightid);
        Query *q = new Query(op_table[op-1], *left, *right);
        return _query->put(q);
    CATCH(-1)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_query_1new__ILjava_lang_String_2Ljava_lang_String_2 (JNIEnv *env, jclass clazz, jint op, jstring strleft, jstring strright) {
    TRY
        const char *c_left = env->GetStringUTFChars(strleft, 0);
        const char *c_right = env->GetStringUTFChars(strright, 0);
        Query *q = new Query(op_table[op-1],
	    string(c_left, env->GetStringUTFLength(strleft)),
	    string(c_right, env->GetStringUTFLength(strright)));
        env->ReleaseStringUTFChars(strleft, c_left);
        env->ReleaseStringUTFChars(strright, c_right);
        return _query->put(q);
    CATCH(-1)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_query_1new__I_3Ljava_lang_String_2 (JNIEnv *env, jclass clazz, jint op, jobjectArray terms) {
    TRY
        jsize size = env->GetArrayLength(terms);
        string *array = toArray(env, terms, size);
        Query *q = new Query(op_table[op-1], array, array+size);
	delete[] array;
        return _query->put(q);
    CATCH(-1)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_query_1new__I_3J (JNIEnv *env, jclass clazz, jint op, jlongArray qids) {
    TRY
	jsize len = env->GetArrayLength(qids);
	Query **queries = new Query*[len];
	jlong *qid_ptr = env->GetLongArrayElements(qids, NULL);
	for (int x=0; x<len; x++) {
	    queries[x] = _query->get(qid_ptr[x]);
	}
        Query *q = new Query(op_table[op-1], queries, queries+len);
	/* We don't change the array so use JNI_ABORT to avoid any work
	 * copying back non-existent changes if the JVM gave us a copy
	 * of the array data. */
	env->ReleaseLongArrayElements(qids, qid_ptr, JNI_ABORT);
	delete[] queries;
        return _query->put(q);
    CATCH(-1)
}

JNIEXPORT jstring JNICALL Java_org_xapian_XapianJNI_query_1get_1description (JNIEnv *env, jclass clazz, jlong qid) {
    TRY
        Query *q = _query->get(qid);
        return env->NewStringUTF(q->get_description().c_str());
    CATCH(NULL)
}

JNIEXPORT jboolean JNICALL Java_org_xapian_XapianJNI_query_1empty (JNIEnv *env, jclass clazz, jlong qid) {
    TRY
        Query *q = _query->get(qid);
        return q->empty();
    CATCH(false)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_query_1terms_1begin (JNIEnv *env, jclass clazz, jlong qid) {
    TRY
        Query *q = _query->get(qid);
        TermIterator *itr = new TermIterator(q->get_terms_begin());
        return _termiterator->put(itr);
    CATCH(-1)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_query_1terms_1end (JNIEnv *env, jclass clazz, jlong qid) {
    TRY
        Query *q = _query->get(qid);
        TermIterator *itr = new TermIterator(q->get_terms_end());
        return _termiterator->put(itr);
    CATCH(-1)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_query_1get_1length (JNIEnv *env, jclass clazz, jlong qid) {
    TRY
        Query *q = _query->get(qid);
        return q->get_length();
    CATCH(-1)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_query_1finalize (JNIEnv *env, jclass clazz, jlong qid) {
    Query *q = _query->remove(qid);
    if (q) delete q;
}
