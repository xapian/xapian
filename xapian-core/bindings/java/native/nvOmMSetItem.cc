// C++ includes
#include <string>
#include <vector>

// Open Muscat includes
#include "om/om.h"

// JNI includes
#include "com_muscat_om_OmMSetItem.h"
#include "utils.h"

/*
 * Class:     com_muscat_om_OmMSetItem
 * Method:    deleteNativeObject
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmMSetItem_deleteNativeObject
  (JNIEnv *env, jobject obj)
{
    delete (OmMSetItem*) tryGetLongField (env, obj, "nativePtr");
}

/*
 * Class:     com_muscat_om_OmMSetItem
 * Method:    getDid
 * Signature: ()J
 */
JNIEXPORT jint JNICALL Java_com_muscat_om_OmMSetItem_get_1did

  (JNIEnv *env, jobject obj)
{
    return (jint)((OmMSetItem*) tryGetLongField (env, obj, "nativePtr"))->did;
}

/*
 * Class:     com_muscat_om_OmMSetItem
 * Method:    getWt
 * Signature: ()D
 */
JNIEXPORT jdouble JNICALL Java_com_muscat_om_OmMSetItem_get_1wt
  (JNIEnv *env, jobject obj)
{
    return (jdouble)((OmMSetItem*) tryGetLongField (env, obj, "nativePtr"))->wt;
}


/*
 * Class:     com_muscat_om_OmMSetItem
 * Method:    get_collapse_key
 * Signature: ()Lcom/muscat/om/OmKey;
 */
JNIEXPORT jobject JNICALL Java_com_muscat_om_OmMSetItem_get_1collapse_1key
  (JNIEnv *env, jobject obj)
{
    OmKey key = ((OmMSetItem*) tryGetLongField (env, obj, "nativePtr"))->collapse_key;

    //cerr << "--- nvOmMSetItem: collapse_key=" << key.value << ";\n";

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
