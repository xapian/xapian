// C++ includes

// Open Muscat includes
#include "om/om.h"
#include "om/omenquire.h"

// JNI includes
#include "com_muscat_om_OmExpandOptions.h"
#include "utils.h"

/*
 * Class:     com_muscat_om_OmExpandOptions
 * Method:    createNativeObject
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_muscat_om_OmExpandOptions_createNativeObject
    (JNIEnv *env, jobject obj) 
{
    return (jlong) new OmExpandOptions ();        
}

/*
 * Class:     com_muscat_om_OmExpandOptions
 * Method:    deleteNativeObject
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmExpandOptions_deleteNativeObject
  (JNIEnv *env, jobject obj)
{
    delete (OmExpandOptions*) tryGetLongField (env, obj, "nativePtr");
}

/*
 * Class:     com_muscat_om_OmExpandOptions
 * Method:    use_query_terms
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmExpandOptions_use_1query_1terms
  (JNIEnv *env, jobject obj, jboolean use)
{
    OmExpandOptions* eoptions = (OmExpandOptions*) tryGetLongField (env, obj, "nativePtr");
    eoptions->use_query_terms ((bool) use);
}
