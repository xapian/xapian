/**
 Copyright (c) 2003, Technology Concepts & Design, Inc.
 Copyright (c) 2006,2011, Olly Betts
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

#ifndef __XAPIAN_JNI_H__
#define __XAPIAN_JNI_H__

#include <jni.h>

// Disable any deprecation warnings for Xapian methods/functions/classes.
#define XAPIAN_DEPRECATED(D) D
#include <xapian.h>

#include "org_xapian_XapianJNI.h"

using namespace std;
using namespace Xapian;


//
// Macros for dealing with C++ exceptions
//

#define TRY try {

//
// when catching an exception, the first thing we want to do
// is see if we have a pending java exception to deal with.
// if not, we we need to translate any Xapian::Error we might
// have caught into the Java equivalant.
#define CATCH(_rc_) \
                    check_for_java_exception(env);         \
                } catch(const char *message) { \
                    if (check_for_java_exception(env)) { return _rc_; } \
                    env->ThrowNew(env->FindClass("java/lang/RuntimeException"), message); \
                } catch(const Xapian::Error &error) { \
                    if (check_for_java_exception(env)) { return _rc_; } \
		    string classname("org/xapian/errors/"); \
		    classname += error.get_type(); \
		    env->ThrowNew(env->FindClass(classname.c_str()), error.get_msg().c_str()); \
                } catch(...) { \
                    if (check_for_java_exception(env)) { return _rc_; } \
                    env->ThrowNew(env->FindClass("java/lang/RuntimeException"), "Unknown error occurred"); \
                } \
                return _rc_;

//
// function declarations
//

/**
 * Checks to see if we have a pending Java exception to deal with.
 * If yes, then we "throw" it, and return true.  Otherwise, we return false.
 */
extern bool check_for_java_exception(JNIEnv *env);

/**
 * takes a java object array (of Strings), and converts it into a C++ string array
 */
extern string *toArray(JNIEnv *env, jobjectArray j_array, int len);

#define CompileTimeAssert(COND)\
    do {\
        typedef int xapian_compile_time_check_[(COND) ? 1 : -1];\
    } while (0)

template <class T> jlong id_from_obj(T obj) {
    // jlong needs to be at least as last as the pointer T to hold its value
    // without losing information.
    CompileTimeAssert(sizeof(T) <= sizeof(jlong));
    // Make sure that NULL casts to 0, since we use 0 on the Java side to
    // represent "no pointer set".  If this is an issue on any platform, we
    // could subtract reinterpret_cast of NULL, but we need to have an unsigned
    // version of jlong in case this subtraction overflows.
    CompileTimeAssert(reinterpret_cast<jlong>(T(0)) == 0);
    return reinterpret_cast<jlong>(obj);
}

template <class T> T obj_from_id(jlong id) {
    return reinterpret_cast<T>(id);
}

inline string cpp_string(JNIEnv *env, jstring s) {
    const char *c_string = env->GetStringUTFChars(s, 0);
    string result(c_string, env->GetStringUTFLength(s));
    env->ReleaseStringUTFChars(s, c_string);
    return result;
}

#endif
