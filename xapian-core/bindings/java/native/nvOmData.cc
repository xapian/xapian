// C++ includes
#include <string>

// Open Muscat includes
#include "om/om.h"

// JNI includes
#include "com_muscat_om_OmData.h"
#include "utils.h"

/*
 * Class:     com_muscat_om_OmData
 * Method:    deleteNativeObject
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmData_deleteNativeObject
  (JNIEnv *env, jobject obj)
{
    delete (OmData*) tryGetLongField (env, obj, "nativePtr");
}


/*
 * Class:     com_muscat_om_OmData
 * Method:    get_data
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_muscat_om_OmData_get_1value
  (JNIEnv *env, jobject obj)
{
    OmData* dat = (OmData*) tryGetLongField (env, obj, "nativePtr");
    return env->NewStringUTF (dat->value.c_str());
}
