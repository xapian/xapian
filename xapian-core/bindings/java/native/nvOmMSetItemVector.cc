// C++ includes
#include <string>
#include <vector>

// Open Muscat includes
#include "om/om.h"

// JNI includes
#include "com_muscat_om_OmMSetItemVector.h"
#include "utils.h"

/*
 * Class:     com_muscat_om_OmMSetItemVector
 * Method:    deleteNativeObject
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmMSetItemVector_deleteNativeObject
  (JNIEnv *env, jobject obj)
{
    delete (vector<OmMSetItem>*) tryGetLongField (env, obj, "nativePtr");    
}

/*
 * Class:     com_muscat_om_OmMSetItemVector
 * Method:    size
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_muscat_om_OmMSetItemVector_size
  (JNIEnv *env, jobject obj)
{
    vector<OmMSetItem>* vec = (vector<OmMSetItem>*) tryGetLongField (env, obj, "nativePtr");
    return vec->size();
}

/*
 * Class:     com_muscat_om_OmMSetItemVector
 * Method:    elementAt
 * Signature: (I)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_com_muscat_om_OmMSetItemVector_elementAt
  (JNIEnv *env, jobject obj, jint pos)
{
    vector<OmMSetItem>* vec = (vector<OmMSetItem>*) tryGetLongField (env, obj, "nativePtr");
    OmMSetItem* item = new OmMSetItem ((*vec)[pos]);
    return makeReturnObject (env, "com/muscat/om/OmMSetItem", (jlong) item);
}
