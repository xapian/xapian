#include <string>
#include <jni.h>
#include "utils.h"
#include "om/om.h"

void throwNewException (JNIEnv *env, const char* type, const char* msg) {
    jclass xclass = env->FindClass (type);
    if (xclass != NULL) 
	env->ThrowNew (xclass, msg);
    
    env->DeleteLocalRef (xclass);
}

void handleNativeError (JNIEnv *env, OmError& err_ref) {
    // ick! ugly code, but hard to see how it could be improved

    const char* msg = err_ref.get_msg().c_str();

    OmError* err = &err_ref;

    if (OmError* err1 = dynamic_cast<OmRuntimeError*>(err)) {
	if (OmError* err2 = dynamic_cast<OmOpeningError*>(err1)) {
	    throwNewException (env, "com/muscat/om/OmOpeningError", msg);
	}
	else if (OmError* err2 = dynamic_cast<OmDocNotFoundError*>(err1)) {
	    throwNewException (env, "com/muscat/om/OmDocNotFoundError", msg);
	}
	else if (OmError* err2 = dynamic_cast<OmNetworkError*>(err1)) {
	    throwNewException (env, "com/muscat/om/OmNetworkError", msg);
	}
	else {
	    throwNewException (env, "com/muscat/om/OmRuntimeError", msg);
	}
    }
    else if (OmError* err1 = dynamic_cast<OmLogicError*>(err)) {
	if (OmError* err2 = dynamic_cast<OmUnimplementedError*>(err1)) {
	    throwNewException (env, "com/muscat/om/OmUnimplementedError", msg);
	}
	else if (OmError* err2 = dynamic_cast<OmInvalidArgumentError*>(err1)) {
	    throwNewException (env, "com/muscat/om/OmInvalidArgumentError", msg);
	}
	else if (OmError* err2 = dynamic_cast<OmAssertionError*>(err1)) {
	    throwNewException (env, "com/muscat/om/OmInvalidArgumentError", msg);
	}
	else {
	    throwNewException (env, "com/muscat/om/OmRuntimeError", msg);
	}
    }
    else {
	throwNewException (env, "com/muscat/om/OmError", msg);
    }
}

jobject makeReturnObject (JNIEnv* env, const char* classname, jlong native) {
    jclass clazz = env->FindClass (classname);
    if (clazz == NULL) {
	string msg = string ("NATIVE: error getting class for: ") + classname;
	throwNewException (env, "java/lang/RuntimeException", msg.c_str());
    }

    jmethodID cid = env->GetMethodID (clazz, "<init>", "(J)V"); 
    if (cid == NULL) {
	string msg = string ("NATIVE: error getting constructor ID for: ") + classname;
	throwNewException (env, "java/lang/RuntimeException", msg.c_str());
    }
    
    jobject ret = env->NewObject (clazz, cid, native);
    env->DeleteLocalRef (clazz);
    return ret;
}

jlong tryGetLongField (JNIEnv* env, const jobject& obj, const char* fieldname) {
    jfieldID fid = env->GetFieldID (env->GetObjectClass (obj), fieldname, "J");
    if (fid == NULL) {
	string msg = string ("NATIVE: error getting long field: ") + fieldname;
	throwNewException (env, "java/lang/RuntimeException", msg.c_str());
    }
    
    jlong ret = env->GetLongField (obj, fid);
    return ret;
}

jdouble tryGetDoubleField (JNIEnv* env, const jobject& obj, const char* fieldname) {
    jfieldID fid = env->GetFieldID (env->GetObjectClass (obj), fieldname, "J");
    if (fid == NULL) {
	string msg = string ("NATIVE: error getting double field: ") + fieldname;
	throwNewException (env, "java/lang/RuntimeException", msg.c_str());
    }

    jdouble ret = env->GetDoubleField (obj, fid);
    return ret;
}

string getStringValue (JNIEnv* env, const jstring& str) {
    const char* ptr = env->GetStringUTFChars (str, NULL);
    string ret (ptr);
    env->ReleaseStringUTFChars (str, ptr);
    return ret;
}
