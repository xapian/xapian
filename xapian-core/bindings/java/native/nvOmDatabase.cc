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
JNIEXPORT jlong JNICALL Java_com_muscat_om_OmDatabase_createNativeObject__Lcom_muscat_om_OmSettings_2
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
 * Method:    createNativeObject
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_muscat_om_OmDatabase_createNativeObject__
  (JNIEnv *env, jobject obj)
{
    try {
	return (jlong) new OmDatabase();
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
 * Method:    add_database
 * Signature: (Lcom/muscat/om/OmSettings;)V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmDatabase_add_1database__Lcom_muscat_om_OmSettings_2
  (JNIEnv *env, jobject obj, jobject param)
{
    OmDatabase* db_n = (OmDatabase*) tryGetLongField (env, obj, "nativePtr");
    OmSettings* param_n = (OmSettings*) tryGetLongField(env, param, "nativePtr");
    try {
	db_n->add_database(*param_n);
    } catch (OmError &err) {
	handleNativeError (env, err);
    }
}

/*
 * Class:     com_muscat_om_OmDatabase
 * Method:    add_database
 * Signature: (Lcom/muscat/om/OmDatabase;)V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmDatabase_add_1database__Lcom_muscat_om_OmDatabase_2
  (JNIEnv *env, jobject obj, jobject db)
{
    OmDatabase* db_n = (OmDatabase*) tryGetLongField (env, obj, "nativePtr");
    OmDatabase* db2_n = (OmDatabase*) tryGetLongField(env, db, "nativePtr");
    try {
	db_n->add_database(*db2_n);
    } catch (OmError &err) {
	handleNativeError (env, err);
    }
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
