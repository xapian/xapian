// C++ includes
#include <string>
#include <vector>

// Open Muscat includes
#include "om/om.h"

// JNI includes
#include "com_muscat_om_OmESetItem.h"
#include "utils.h"

/*
 * Class:     com_muscat_om_OmESetItem
 * Method:    deleteNativeObject
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmESetItem_deleteNativeObject
  (JNIEnv *env, jobject obj)
{
    delete (OmESetItem*) tryGetLongField (env, obj, "nativePtr");        
}

/*
 * Class:     com_muscat_om_OmESetItem
 * Method:    get_tname
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_muscat_om_OmESetItem_get_1tname
  (JNIEnv *env, jobject obj)
{
    OmESetItem* item = (OmESetItem*) tryGetLongField (env, obj, "nativePtr");
    return env->NewStringUTF (item->tname.c_str());    
}

/*
 * Class:     com_muscat_om_OmESetItem
 * Method:    get_wt
 * Signature: ()D
 */
JNIEXPORT jdouble JNICALL Java_com_muscat_om_OmESetItem_get_1wt
  (JNIEnv *env, jobject obj)
{
    return (jdouble) ((OmESetItem*) tryGetLongField (env, obj, "nativePtr")) -> wt;
}
