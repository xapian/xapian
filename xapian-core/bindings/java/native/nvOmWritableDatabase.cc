
// Open Muscat includes
#include "om/om.h"

// JNI includes
#include "com_muscat_om_OmWritableDatabase.h"
#include "utils.h"

/*
 * Class:     com_muscat_om_OmWritableDatabase
 * Method:    createNativeObject
 * Signature: (Lcom/muscat/om/OmSettings;)J
 */
JNIEXPORT jlong JNICALL Java_com_muscat_om_OmWritableDatabase_createNativeObject
  (JNIEnv *env, jobject obj, jobject params)
{
    const OmSettings *params_n = (const OmSettings *) tryGetLongField (env, params, "nativePtr");

    try {
	return (jlong) new OmWritableDatabase(*params_n);
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
 * Method:    begin_session
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmWritableDatabase_begin_1session
  (JNIEnv *env, jobject obj, jint timeout)
{
    OmWritableDatabase *db = (OmWritableDatabase *) tryGetLongField (env, obj, "nativePtr");
    try {
	db->begin_session((om_timeout)timeout);
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }
}

/*
 * Class:     com_muscat_om_OmWritableDatabase
 * Method:    end_session
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmWritableDatabase_end_1session
  (JNIEnv *env, jobject obj)
{
    OmWritableDatabase *db = (OmWritableDatabase *) tryGetLongField (env, obj, "nativePtr");
    try {
	db->end_session();
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
//	cerr << db->get_description() << endl;
//	cerr << doc->get_description() << endl;
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
