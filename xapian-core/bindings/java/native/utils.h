#ifndef OM_HGUARD_JAVA_NATIVE_UTILS_H
#define OM_HGUARD_JAVA_NATIVE_UTILS_H

#include "om/om.h"

/* Provide strcasecmp if not around. */
#ifndef HAVE_STRCASECMP
int strcasecmp(const char *s1, const char *s2);
#endif

void    throwNewException (JNIEnv* env, const char* type, const char* msg);
void    handleNativeError (JNIEnv* env, OmError& err);
jobject makeReturnObject  (JNIEnv* env, const char* classname, jlong native);
jlong   tryGetLongField   (JNIEnv* env, const jobject& obj, const char* fieldname);
jdouble tryGetDoubleField (JNIEnv* env, const jobject& obj, const char* fieldname);
string  getStringValue    (JNIEnv* env, const jstring& str);

#endif /* OM_HGUARD_JAVA_NATIVE_UTILS_H */
