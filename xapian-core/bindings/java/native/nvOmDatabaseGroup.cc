// C++ includes
#include <string>
#include <vector>
#include <iostream>

// Open Muscat includes
#include "om/om.h"
#include "om/omenquire.h"

// JNI includes
#include "com_muscat_om_OmDatabaseGroup.h"
#include "utils.h"

/*
 * Class:     com_muscat_om_OmDatabaseGroup
 * Method:    createNativeObject
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_muscat_om_OmDatabaseGroup_createNativeObject
  (JNIEnv *env, jobject obj)
{
    return (jlong) new OmDatabaseGroup ();
}

/*
 * Class:     com_muscat_om_OmDatabaseGroup
 * Method:    deleteNativeObject
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmDatabaseGroup_deleteNativeObject
  (JNIEnv *env, jobject obj)
{
    delete (OmDatabaseGroup*) tryGetLongField (env, obj, "nativePtr");
}

/*
 * Class:     com_muscat_om_OmDatabaseGroup
 * Method:    add_database
 * Signature: (Ljava/lang/String;[Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmDatabaseGroup_add_1database
  (JNIEnv *env, jobject obj, jstring type, jobjectArray params)
{
    OmDatabaseGroup* db = (OmDatabaseGroup*) tryGetLongField (env, obj, "nativePtr");
    string type_n = getStringValue (env, type);

    jsize params_len = env->GetArrayLength (params);
    vector<string> params_n;
    for (jsize i = 0; i < params_len; i++) {
	jobject param_obj = env->GetObjectArrayElement (params, i);
	string parm = getStringValue (env, (jstring) param_obj);
	params_n.push_back (parm);
	env->DeleteLocalRef (param_obj);
    }

    try {
	db->add_database (type_n, params_n);
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }
}
