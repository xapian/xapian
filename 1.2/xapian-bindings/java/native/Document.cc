/**
 Copyright (c) 2003, Technology Concepts & Design, Inc.
 Copyright (c) 2008,2011, Olly Betts
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

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_document_1new__ (JNIEnv *env, jclass clazz) {
    TRY
        return id_from_obj(new Document());
    CATCH(-1)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_document_1new__J (JNIEnv *env, jclass clazz, jlong docid) {
    TRY
        Document *existing = obj_from_id<Document *>(docid);
        return id_from_obj(new Document(*existing));
    CATCH(-1)
}


JNIEXPORT jstring JNICALL Java_org_xapian_XapianJNI_docment_1get_1value (JNIEnv *env, jclass clazz, jlong docid, jint value_index) {
    TRY
        Document *doc = obj_from_id<Document *>(docid);
        string value = doc->get_value(value_index);
        if (value.c_str()) {
            return env->NewStringUTF(value.c_str());
        } else {
            return 0;
        }
    CATCH(NULL)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_document_1add_1value (JNIEnv *env, jclass clazz, jlong docid, jint value_index, jstring value) {
    TRY
        Document *doc = obj_from_id<Document *>(docid);
        doc->add_value(value_index, cpp_string(env, value));
    CATCH(;)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_document_1remove_1value (JNIEnv *env, jclass clazz, jlong docid, jint value_index) {
    TRY
        Document *doc = obj_from_id<Document *>(docid);
        doc->remove_value(value_index);
    CATCH(;)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_document_1clear_1values (JNIEnv *env, jclass clazz, jlong docid) {
    TRY
        Document *doc = obj_from_id<Document *>(docid);
        doc->clear_values();
    CATCH(;)
}

JNIEXPORT jstring JNICALL Java_org_xapian_XapianJNI_document_1get_1data (JNIEnv *env, jclass clazz, jlong docid) {
    TRY
        Document *doc = obj_from_id<Document *>(docid);
        return env->NewStringUTF(doc->get_data().c_str());
    CATCH(NULL)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_document_1set_1data (JNIEnv *env, jclass clazz, jlong docid, jstring data) {
    TRY
        Document *doc = obj_from_id<Document *>(docid);
        doc->set_data(cpp_string(env, data));
    CATCH(;)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_document_1add_1posting (JNIEnv *env, jclass clazz, jlong docid, jstring term, jint position) {
    TRY
        Document *doc = obj_from_id<Document *>(docid);
        doc->add_posting(cpp_string(env, term), position);
    CATCH(;)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_document_1add_1term (JNIEnv *env, jclass clazz, jlong docid, jstring term) {
    TRY
        Document *doc = obj_from_id<Document *>(docid);
        doc->add_term(cpp_string(env, term));
    CATCH(;)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_document_1remove_1posting (JNIEnv *env, jclass clazz, jlong docid, jstring term, jint position) {
    TRY
        Document *doc = obj_from_id<Document *>(docid);
        doc->remove_posting(cpp_string(env, term), position);
    CATCH(;)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_document_1remove_1term (JNIEnv *env, jclass clazz, jlong docid, jstring term) {
    TRY
        Document *doc = obj_from_id<Document *>(docid);
        doc->remove_term(cpp_string(env, term));
    CATCH(;)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_document_1clear_1terms (JNIEnv *env, jclass clazz, jlong docid) {
    TRY
        Document *doc = obj_from_id<Document *>(docid);
        doc->clear_terms();
    CATCH(;)
}

JNIEXPORT jint JNICALL Java_org_xapian_XapianJNI_document_1termlist_1count (JNIEnv *env, jclass clazz, jlong docid) {
    TRY
        Document *doc = obj_from_id<Document *>(docid);
        return doc->termlist_count();
    CATCH(-1)
}

JNIEXPORT jint JNICALL Java_org_xapian_XapianJNI_document_1values_1count (JNIEnv *env, jclass clazz, jlong docid) {
    TRY
        Document *doc = obj_from_id<Document *>(docid);
        return doc->values_count();
    CATCH(-1)
}

JNIEXPORT jstring JNICALL Java_org_xapian_XapianJNI_document_1get_1description (JNIEnv *env, jclass clazz, jlong docid) {
    TRY
        Document *doc = obj_from_id<Document *>(docid);
        return env->NewStringUTF(doc->get_description().c_str());
    CATCH(NULL)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_document_1finalize (JNIEnv *env, jclass clazz, jlong docid) {
    Document *doc = obj_from_id<Document *>(docid);
    delete doc;
}
