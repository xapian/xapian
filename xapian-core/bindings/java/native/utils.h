#ifndef __utils_h__
#define __utils_h__

#include "om.h"

void    throwNewException (JNIEnv* env, const char* type, const char* msg);
void    handleNativeError (JNIEnv* env, OmError& err);
jobject makeReturnObject  (JNIEnv* env, const char* classname, jlong native);
jlong   tryGetLongField   (JNIEnv* env, const jobject& obj, const char* fieldname);
jdouble tryGetDoubleField (JNIEnv* env, const jobject& obj, const char* fieldname);
string  getStringValue    (JNIEnv* env, const jstring& str);

#endif
