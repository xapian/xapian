// C++ includes
#include <vector>

// Open Muscat includes
#include "om/om.h"

// JNI includes
#include "com_muscat_om_OmESet.h"
#include "utils.h"

/*
 * Class:     com_muscat_om_OmESet
 * Method:    deleteNativeObject
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmESet_deleteNativeObject
  (JNIEnv *env, jobject obj)
{
    delete (OmESet*) tryGetLongField (env, obj, "nativePtr");
}

/*
 * Class:     com_muscat_om_OmESet
 * Method:    get_items
 * Signature: ()Lcom/muscat/om/OmVector;
 */
JNIEXPORT jobject JNICALL Java_com_muscat_om_OmESet_get_1items
  (JNIEnv *env, jobject obj)
{
    OmESet* eset = (OmESet*) tryGetLongField (env, obj, "nativePtr");
    jlong native = (jlong) new vector<OmESetItem> (eset->items);
    return makeReturnObject (env, "com/muscat/om/OmESetItemVector", native);
}

/*
 * Class:     com_muscat_om_OmESet
 * Method:    get_ebound
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_muscat_om_OmESet_get_1ebound
  (JNIEnv *env, jobject obj)
{
    OmESet* eset = (OmESet*) tryGetLongField (env, obj, "nativePtr");
    return (jint) eset->ebound;
}
