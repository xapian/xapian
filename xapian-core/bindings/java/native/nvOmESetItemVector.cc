// C++ includes
#include <string>
#include <vector>

// Open Muscat includes
#include "om/om.h"

// JNI includes
#include "com_muscat_om_OmESetItemVector.h"
#include "utils.h"

/*
 * Class:     com_muscat_om_OmESetItemVector
 * Method:    deleteNativeObject
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmESetItemVector_deleteNativeObject
  (JNIEnv *env, jobject obj)
{
    delete (vector<OmESetItem>*) tryGetLongField (env, obj, "nativePtr");
}

/*
 * Class:     com_muscat_om_OmESetItemVector
 * Method:    size
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_muscat_om_OmESetItemVector_size
  (JNIEnv *env, jobject obj)
{
    vector<OmESetItem>* vec = (vector<OmESetItem>*) tryGetLongField (env, obj, "nativePtr");
    return vec->size();
}

/*
 * Class:     com_muscat_om_OmESetItemVector
 * Method:    elementAt
 * Signature: (I)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_com_muscat_om_OmESetItemVector_elementAt
  (JNIEnv *env, jobject obj, jint pos)
{
    vector<OmESetItem>* vec = (vector<OmESetItem>*) tryGetLongField (env, obj, "nativePtr");
    OmESetItem* item = new OmESetItem ((*vec)[pos]);
    return makeReturnObject (env, "com/muscat/om/OmESetItem", (jlong) item);
}
