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
 
#include <jni.h>
#include <typeinfo>
#include "xapian_jni.h"

bool check_for_java_exception(JNIEnv *env) {
    jthrowable exception = env->ExceptionOccurred();
    if (exception) {
        env->ExceptionClear();
        env->Throw(exception);
        return true;
    }

    return false;
}

void setup_errormap() {
    _errormap[typeid(Xapian::Error).name()] = "org/xapian/errors/XapianError";
    _errormap[typeid(Xapian::LogicError).name()] = "org/xapian/errors/LogicError";
    _errormap[typeid(Xapian::RuntimeError).name()] = "org/xapian/errors/RuntimeError";
    _errormap[typeid(Xapian::AssertionError).name()] = "org/xapian/errors/AssertionError";
    _errormap[typeid(Xapian::UnimplementedError).name()] = "org/xapian/errors/UnimplementedError";
    _errormap[typeid(Xapian::InvalidArgumentError).name()] = "org/xapian/errors/InvalidArgumentError";
    _errormap[typeid(Xapian::InvalidOperationError).name()] = "org/xapian/errors/InvalidOperationError";
    _errormap[typeid(Xapian::DocNotFoundError).name()] = "org/xapian/errors/DocNotFoundError";
    _errormap[typeid(Xapian::RangeError).name()] = "org/xapian/errors/RangeError";
    _errormap[typeid(Xapian::InternalError).name()] = "org/xapian/errors/InternalError";
    _errormap[typeid(Xapian::DatabaseError).name()] = "org/xapian/errors/DatabaseError";
    _errormap[typeid(Xapian::FeatureUnavailableError).name()] = "org/xapian/errors/FeatureUnavailableError";
    _errormap[typeid(Xapian::NetworkError).name()] = "org/xapian/errors/NetworkError";
    _errormap[typeid(Xapian::NetworkTimeoutError).name()] = "org/xapian/errors/NetworkTimeoutError";
    _errormap[typeid(Xapian::DatabaseCorruptError).name()] = "org/xapian/errors/DatabaseCorruptError";
    _errormap[typeid(Xapian::DatabaseCreateError).name()] = "org/xapian/errors/DatabaseCreateError";
    _errormap[typeid(Xapian::DatabaseOpeningError).name()] = "org/xapian/errors/DatabaseOpeningError";
    _errormap[typeid(Xapian::DatabaseLockError).name()] = "org/xapian/errors/DatabaseLockError";
    _errormap[typeid(Xapian::DatabaseModifiedError).name()] = "org/xapian/errors/DatabaseModifiedError";
    _errormap[typeid(Xapian::InvalidResultError).name()] = "org/xapian/errors/InvalidResultError";
    _errormap[typeid(Xapian::TypeError).name()] = "org/xapian/errors/TypeError";
    _errormap[typeid(Xapian::InvalidDataError).name()] = "org/xapian/errors/InvalidDataError";
    _errormap[typeid(Xapian::DataFlowError).name()] = "org/xapian/errors/DataFlowError";
}

string *toArray(JNIEnv *env, jobjectArray j_array, int len) {
    string *array = new string[len];
    for (int x=0; x<len; x++) {
        jstring term = (jstring) env->GetObjectArrayElement(j_array, x);
        const char *c_term = env->GetStringUTFChars(term, 0);
        array[x] = c_term;
        env->ReleaseStringUTFChars(term, c_term);
    }
    return array;
}
