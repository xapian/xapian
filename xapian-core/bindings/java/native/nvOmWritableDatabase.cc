
// Open Muscat includes
#include "om/om.h"

// JNI includes
#include "com_muscat_om_OmWritableDatabase.h"
#include "utils.h"

/*
 * Class:     com_muscat_om_OmWritableDatabase
 * Method:    createNativeObject
 * Signature: (Ljava/lang/String;[Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_com_muscat_om_OmWritableDatabase_createNativeObject
  (JNIEnv *env, jobject obj, jstring type, jobjectArray params)
{
    std::string type_n = getStringValue (env, type);

    jsize params_len = env->GetArrayLength (params);
    std::vector<string> params_n;
    for (jsize i = 0; i < params_len; i++) {
	jobject param_obj = env->GetObjectArrayElement (params, i);
	std::string parm = getStringValue (env, (jstring) param_obj);
	params_n.push_back (parm);
	env->DeleteLocalRef (param_obj);
    }

    try {
	return (jlong) new OmWritableDatabase(type_n, params_n);
    } catch (OmError &err) {
	handleNativeError (env, err);
    }
}

/*
 * Class:     com_muscat_om_OmWritableDatabase
 * Method:    deleteNativeObject
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmWritableDatabase_deleteNativeObject
  (JNIEnv *env, jobject obj)
{
    delete (OmWritableDatabase *) tryGetLongField (env, obj, "nativePtr");
}

/*
 * Class:     com_muscat_om_OmWritableDatabase
 * Method:    lock
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmWritableDatabase_lock
  (JNIEnv *env, jobject obj, jint timeout)
{
    OmWritableDatabase *db = (OmWritableDatabase *) tryGetLongField (env, obj, "nativePtr");
    try {
	db->lock((om_timeout)timeout);
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }
}

/*
 * Class:     com_muscat_om_OmWritableDatabase
 * Method:    unlock
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmWritableDatabase_unlock
  (JNIEnv *env, jobject obj)
{
    OmWritableDatabase *db = (OmWritableDatabase *) tryGetLongField (env, obj, "nativePtr");
    try {
	db->unlock();
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }
}

/*
 * Class:     com_muscat_om_OmWritableDatabase
 * Method:    add_document
 * Signature: (Lcom/muscat/om/OmDocumentContents;)I
 */
JNIEXPORT jint JNICALL Java_com_muscat_om_OmWritableDatabase_add_1document
  (JNIEnv *env, jobject obj, jobject document)
{
    OmWritableDatabase *db = (OmWritableDatabase *) tryGetLongField (env, obj, "nativePtr");
    OmDocumentContents *doc = (OmDocumentContents *) tryGetLongField (env, document, "nativePtr");
    try {
	return (jint)db->add_document(*doc);
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }
}

/*
 * Class:     com_muscat_om_OmWritableDatabase
 * Method:    get_description
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_muscat_om_OmWritableDatabase_get_1description
  (JNIEnv *env, jobject obj)
{
    OmWritableDatabase *db = (OmWritableDatabase *) tryGetLongField (env, obj, "nativePtr");
    try {
	return env->NewStringUTF (db->get_description().c_str());
    } catch (OmError &err) {
	handleNativeError (env, err);
    }
    return NULL;
}
