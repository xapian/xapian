// C++ includes
#include <string>
#include <vector>

// Open Muscat includes
#include "om/om.h"

// JNI includes
#include "com_muscat_om_OmMSet.h"
#include "utils.h"

/*
 * Class:     com_muscat_om_OmMSet
 * Method:    deleteNativeObject
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmMSet_deleteNativeObject
  (JNIEnv *env, jobject obj)
{
    delete (OmMSet*) tryGetLongField (env, obj, "nativePtr");
}

/*
 * Class:     com_muscat_om_OmMSet
 * Method:    get_firstitem
 * Signature: ()J
 */
JNIEXPORT jint JNICALL Java_com_muscat_om_OmMSet_get_1firstitem
  (JNIEnv *env, jobject obj)
{
    OmMSet* mset = (OmMSet*) tryGetLongField (env, obj, "nativePtr");
    return (jint) mset->firstitem;
}

/*
 * Class:     com_muscat_om_OmMSet
 * Method:    get_items
 * Signature: ()[Lcom/muscat/om/OmMSetItem;
 */
JNIEXPORT jobject JNICALL Java_com_muscat_om_OmMSet_get_1items
  (JNIEnv *env, jobject obj)
{
    OmMSet* mset = (OmMSet*) tryGetLongField (env, obj, "nativePtr");
    jlong native = (jlong) new vector<OmMSetItem> (mset->items);
    return makeReturnObject (env, "com/muscat/om/OmMSetItemVector", native);
}

/*
 * Class:     com_muscat_om_OmMSet
 * Method:    get_max_attained
 * Signature: ()D
 */
JNIEXPORT jdouble JNICALL Java_com_muscat_om_OmMSet_get_1max_1attained
  (JNIEnv *env, jobject obj)
{
    OmMSet* mset = (OmMSet*) tryGetLongField (env, obj, "nativePtr");
    return (jdouble) mset->max_attained;
}

/*
 * Class:     com_muscat_om_OmMSet
 * Method:    get_max_possible
 * Signature: ()D
 */
JNIEXPORT jdouble JNICALL Java_com_muscat_om_OmMSet_get_1max_1possible
  (JNIEnv *env, jobject obj)
{
    OmMSet* mset = (OmMSet*) tryGetLongField (env, obj, "nativePtr");
    return (jdouble) mset->max_possible;
}

/*
 * Class:     com_muscat_om_OmMSet
 * Method:    get_mbound
 * Signature: ()J
 */
JNIEXPORT jint JNICALL Java_com_muscat_om_OmMSet_get_1mbound
  (JNIEnv *env, jobject obj)
{
    OmMSet* mset = (OmMSet*) tryGetLongField (env, obj, "nativePtr");
    return (jint) mset->mbound;
}

/*
 * Class:     com_muscat_om_OmMSet
 * Method:    convert_to_percent
 * Signature: (D)I
 */
JNIEXPORT jint JNICALL Java_com_muscat_om_OmMSet_convert_1to_1percent
  (JNIEnv *env, jobject obj, jdouble weight)
{
    OmMSet* mset = (OmMSet*) tryGetLongField (env, obj, "nativePtr");
    return (jint) mset->convert_to_percent ((om_weight) weight);
}

/*
 * Class:     com_muscat_om_OmMSet
 * Method:    get_description
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_muscat_om_OmMSet_get_1description
  (JNIEnv *env, jobject obj)
{
    OmMSet* mset = (OmMSet*) tryGetLongField (env, obj, "nativePtr");
    try {
	return env->NewStringUTF (mset->get_description().c_str());
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }
    return NULL;
}

/*
 * Class:     com_muscat_om_OmMSet
 * Method:    get_termfreq
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_muscat_om_OmMSet_get_1termfreq
  (JNIEnv *env, jobject obj, jstring tname)
{
    OmMSet* mset = (OmMSet*) tryGetLongField (env, obj, "nativePtr");
    try {
	return (jint)(mset->get_termfreq(getStringValue(env, tname)));
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }
    return NULL;
}

/*
 * Class:     com_muscat_om_OmMSet
 * Method:    get_termweight
 * Signature: (Ljava/lang/String;)D
 */
JNIEXPORT jdouble JNICALL Java_com_muscat_om_OmMSet_get_1termweight
  (JNIEnv *env, jobject obj, jstring tname)
{
    OmMSet* mset = (OmMSet*) tryGetLongField (env, obj, "nativePtr");
    try {
	return (jdouble)(mset->get_termweight(getStringValue(env, tname)));
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }
    return NULL;
}

