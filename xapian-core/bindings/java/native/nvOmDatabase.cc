// Open Muscat includes
#include "om/om.h"

// JNI includes
#include "com_muscat_om_OmDatabase.h"
#include "utils.h"

/*
 * Class:     com_muscat_om_OmDatabase
 * Method:    createNativeObject
 * Signature: (Lcom/muscat/om/OmSettings;)J
 */
JNIEXPORT jlong JNICALL Java_com_muscat_om_OmDatabase_createNativeObject
  (JNIEnv *env, jobject obj, jobject params)
{
    const OmSettings *params_n = (const OmSettings *) tryGetLongField (env, params, "nativePtr");

    try {
	return (jlong) new OmDatabase(*params_n);
    } catch (OmError &err) {
	handleNativeError (env, err);
	return -1;
    }
}

/*
 * Class:     com_muscat_om_OmDatabase
 * Method:    deleteNativeObject
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmDatabase_deleteNativeObject
  (JNIEnv *env, jobject obj)
{
    delete (OmDatabase *) tryGetLongField (env, obj, "nativePtr");
}

/*
 * Class:     com_muscat_om_OmDatabase
 * Method:    get_description
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_muscat_om_OmDatabase_get_1description
  (JNIEnv *env, jobject obj)
{
    OmDatabase *db = (OmDatabase *) tryGetLongField (env, obj, "nativePtr");
    try {
	return env->NewStringUTF (db->get_description().c_str());
    } catch (OmError &err) {
	handleNativeError (env, err);
    }
    return NULL;
}
