/**
 Copyright (c) 2003, Technology Concepts & Design, Inc.
 Copyright (c) 2011, Olly Betts
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

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_rset_1new (JNIEnv *env, jclass clazz) {
    TRY
        RSet *rset = new RSet();
        return id_from_obj(rset);
    CATCH(-1)
}
JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_rset_1size (JNIEnv *env, jclass clazz, jlong rsetid) {
    TRY
        RSet *rset = obj_from_id<RSet *>(rsetid);
        return rset->size();
    CATCH(-1)
}
JNIEXPORT jboolean JNICALL Java_org_xapian_XapianJNI_rset_1empty (JNIEnv *env, jclass clazz, jlong rsetid) {
    TRY
        RSet *rset = obj_from_id<RSet *>(rsetid);
        return rset->empty();
    CATCH(false)
}
JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_rset_1add_1document (JNIEnv *env, jclass clazz, jlong rsetid, jlong dbdocid) {
    TRY
        RSet *rset = obj_from_id<RSet *>(rsetid);
        rset->add_document(dbdocid);
    CATCH(;)
}
JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_rset_1add_1document_1via_1msetiterator (JNIEnv *env, jclass clazz, jlong rsetid, jlong msetiteratorid) {
    TRY
        RSet *rset = obj_from_id<RSet *>(rsetid);
        MSetIterator *itr = obj_from_id<MSetIterator *>(msetiteratorid);
        rset->add_document(*itr);
    CATCH(;)
}
JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_rset_1remove_1document (JNIEnv *env, jclass clazz, jlong rsetid, jlong dbdocid) {
    TRY
        RSet *rset = obj_from_id<RSet *>(rsetid);
        rset->remove_document(dbdocid);
    CATCH(;)
}
JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_rset_1remove_1document_1via_1msetiterator (JNIEnv *env, jclass clazz, jlong rsetid, jlong msetiteratorid) {
    TRY
        RSet *rset = obj_from_id<RSet *>(rsetid);
        MSetIterator *itr = obj_from_id<MSetIterator *>(msetiteratorid);
        rset->remove_document(*itr);
    CATCH(;)
}
JNIEXPORT jboolean JNICALL Java_org_xapian_XapianJNI_rset_1contains (JNIEnv *env, jclass clazz, jlong rsetid, jlong dbdocid) {
    TRY    
        RSet *rset = obj_from_id<RSet *>(rsetid);
        return rset->contains(dbdocid);
    CATCH(false)
}
JNIEXPORT jboolean JNICALL Java_org_xapian_XapianJNI_rset_1contains_1via_1msetiterator (JNIEnv *env, jclass clazz, jlong rsetid, jlong msetiteratorid) {
    TRY    
        RSet *rset = obj_from_id<RSet *>(rsetid);
        MSetIterator *itr = obj_from_id<MSetIterator *>(msetiteratorid);
        return rset->contains(*itr);
    CATCH(false)
}
JNIEXPORT jstring JNICALL Java_org_xapian_XapianJNI_rset_1get_1description (JNIEnv *env, jclass clazz, jlong rsetid) {
    TRY    
        RSet *rset = obj_from_id<RSet *>(rsetid);
        return env->NewStringUTF(rset->get_description().c_str());
    CATCH(NULL)
}
JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_rset_1finalize (JNIEnv *env, jclass clazz, jlong rsetid) {
    RSet *rset = obj_from_id<RSet *>(rsetid);
    delete rset;
}
