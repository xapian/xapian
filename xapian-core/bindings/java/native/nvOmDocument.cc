// C++ includes
#include <string>

// Xapian includes
#include "om/om.h"
#include "om/omdocument.h"

// JNI includes
#include "com_muscat_om_OmDocument.h"
#include "utils.h"

/*
 * Class:     com_muscat_om_OmDocument
 * Method:    deleteNativeObject
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmDocument_deleteNativeObject
  (JNIEnv *env, jobject obj)
{
    delete (OmDocument*) tryGetLongField (env, obj, "nativePtr");
}


/*
 * Class:     com_muscat_om_OmDocument
 * Method:    get_key
 * Signature: (I)Lcom/muscat/om/OmKey;
 */
JNIEXPORT jobject JNICALL Java_com_muscat_om_OmDocument_get_1key
  (JNIEnv *env, jobject obj, jint keyno)
{
    OmDocument* doc = (OmDocument*) tryGetLongField (env, obj, "nativePtr");
    OmKey key = doc->get_key (keyno);

    // convert to Java object

    jclass clazz = env->FindClass ("com/muscat/om/OmKey");
    if (clazz == NULL)
	throwNewException (env, "java/lang/RuntimeException",
			   "NATIVE: error getting class ID for com/muscat/om/OmKey");

    jmethodID cid = env->GetMethodID (clazz, "<init>", "(Ljava/lang/String;)V");
    if (cid == NULL)
	throwNewException (env, "java/lang/RuntimeException",
			   "NATIVE: error getting constructor ID for com/muscat/om/OmKey");

    jobject ret =  env->NewObject (clazz, cid,
				   env->NewStringUTF (key.value.c_str()));
    env->DeleteLocalRef (clazz);
    return ret;
}

/*
 * Class:     com_muscat_om_OmDocument
 * Method:    get_data
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jobject JNICALL Java_com_muscat_om_OmDocument_get_1data
  (JNIEnv *env, jobject obj)
{
    OmDocument* doc = (OmDocument*) tryGetLongField (env, obj, "nativePtr");
    return env->NewStringUTF(doc->get_data().c_str());
}
