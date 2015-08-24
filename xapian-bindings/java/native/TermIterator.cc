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

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_termiterator_1positionlist_1begin (JNIEnv *env, jclass clazz, jlong termiteratorid) {
    TRY
        TermIterator *itr = obj_from_id<TermIterator *>(termiteratorid);
        PositionIterator *positr = new PositionIterator(itr->positionlist_begin());
        return id_from_obj(positr);
    CATCH(-1)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_termiterator_1positionlist_1end (JNIEnv *env, jclass clazz, jlong termiteratorid) {
    TRY
        TermIterator *itr = obj_from_id<TermIterator *>(termiteratorid);
        PositionIterator *positr = new PositionIterator(itr->positionlist_end());
        return id_from_obj(positr);
    CATCH(-1)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_termiterator_1next (JNIEnv *env, jclass clazz, jlong termiteratorid) {
    TRY
        TermIterator *itr = obj_from_id<TermIterator *>(termiteratorid);
        (*itr)++;
    CATCH(;)
}

JNIEXPORT jstring JNICALL Java_org_xapian_XapianJNI_termiterator_1get_1termname (JNIEnv *env, jclass clazz, jlong termiteratorid) {
    TRY
        TermIterator *itr = obj_from_id<TermIterator *>(termiteratorid);
        if (!itr) throw "TermIterator is invalid";
        string term = *(*itr);
        return env->NewStringUTF(term.c_str());
    CATCH(NULL)
}

JNIEXPORT jint JNICALL Java_org_xapian_XapianJNI_termiterator_1get_1term_1freq (JNIEnv *env, jclass clazz, jlong termiteratorid) {
    TRY
        TermIterator *itr = obj_from_id<TermIterator *>(termiteratorid);
        return itr->get_termfreq();
    CATCH(-1)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_termiterator_1get_1wdf (JNIEnv *env, jclass clazz, jlong termiteratorid) {
    TRY
        TermIterator *itr = obj_from_id<TermIterator *>(termiteratorid);
        return itr->get_wdf();
    CATCH(-1)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_termiterator_1skip_1to (JNIEnv *env, jclass clazz, jlong termiteratorid, jstring term) {
    TRY
        TermIterator *itr = obj_from_id<TermIterator *>(termiteratorid);
        itr->skip_to(cpp_string(env, term));
    CATCH(;)
}

JNIEXPORT jstring JNICALL Java_org_xapian_XapianJNI_termiterator_1get_1description (JNIEnv *env, jclass clazz, jlong termiteratorid) {
    TRY
        TermIterator *itr = obj_from_id<TermIterator *>(termiteratorid);
        return env->NewStringUTF(itr->get_description().c_str());
    CATCH(NULL)
}

JNIEXPORT jboolean JNICALL Java_org_xapian_XapianJNI_termiterator_1equals (JNIEnv *env, jclass clazz, jlong aid, jlong bid) {
    TRY
        TermIterator *a = obj_from_id<TermIterator *>(aid);
        TermIterator *b = obj_from_id<TermIterator *>(bid);
        return (*a) == (*b);
    CATCH(0)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_termiterator_1finalize (JNIEnv *env, jclass clazz, jlong termiteratorid) {
    TermIterator *itr = obj_from_id<TermIterator *>(termiteratorid);
    delete itr;
}
