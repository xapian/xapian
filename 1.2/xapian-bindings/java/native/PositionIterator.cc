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

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_positioniterator_1getvalue (JNIEnv *env, jclass clazz, jlong positioniteratorid) {
    TRY
        PositionIterator *itr = obj_from_id<PositionIterator *>(positioniteratorid);
        return *(*itr);
    CATCH(-1)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_positioniterator_1next (JNIEnv *env, jclass clazz, jlong positioniteratorid) {
    TRY
        PositionIterator *itr = obj_from_id<PositionIterator *>(positioniteratorid);
        (*itr)++;
    CATCH(;)
}

JNIEXPORT jboolean JNICALL Java_org_xapian_XapianJNI_positioniterator_1equals (JNIEnv *env, jclass clazz, jlong aid, jlong bid) {
    TRY
        PositionIterator *a = obj_from_id<PositionIterator *>(aid);
        PositionIterator *b = obj_from_id<PositionIterator *>(bid);
        return (*a) == (*b);
    CATCH(false)
}

JNIEXPORT jstring JNICALL Java_org_xapian_XapianJNI_positioniterator_1get_1description (JNIEnv *env, jclass clazz, jlong positioniteratorid) {
    TRY
        PositionIterator *itr = obj_from_id<PositionIterator *>(positioniteratorid);
        return env->NewStringUTF(itr->get_description().c_str());
    CATCH(NULL)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_positioniterator_1finalize (JNIEnv *env, jclass clazz, jlong positioniteratorid) {
    PositionIterator *itr = obj_from_id<PositionIterator *>(positioniteratorid);
    delete itr;
}
