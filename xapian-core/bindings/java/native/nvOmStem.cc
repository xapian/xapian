// C++ includes
#include <string>
#include <vector>

// Open Muscat includes
#include "om/om.h"
#include "om/omstem.h"

// JNI includes
#include "com_muscat_om_OmStem.h"
#include "utils.h"

/*
 * Class:     com_muscat_om_OmStem
 * Method:    createNativeObject
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_com_muscat_om_OmStem_createNativeObject
  (JNIEnv *env, jobject obj, jstring lang)
{
    try {
	return (jlong) new OmStem (getStringValue (env, lang));
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }
    return 0;
}

/*
 * Class:     com_muscat_om_OmStem
 * Method:    deleteNativeObject
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmStem_deleteNativeObject
  (JNIEnv *env, jobject obj)
{
    delete (OmStem*) tryGetLongField (env, obj, "nativePtr");
}

/*
 * Class:     com_muscat_om_OmStem
 * Method:    stem_word
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_muscat_om_OmStem_stem_1word
  (JNIEnv *env, jobject obj, jstring word)
{
    OmStem* stem = (OmStem*) tryGetLongField (env, obj, "nativePtr");
    try {
	return env->NewStringUTF (stem->stem_word (getStringValue (env, word)).c_str());
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }
    return NULL;
}

///*
// * Class:     com_muscat_om_OmStem
// * Method:    get_available_languages
// * Signature: ()[Ljava/lang/String;
// */
//JNIEXPORT jobjectArray JNICALL Java_com_muscat_om_OmStem_get_1available_1languages
//  (JNIEnv *env, jclass clazz)
//{
//    vector<string> langs = OmStem::get_available_languages ();
//    jclass strClass = env->FindClass ("java/lang/String");
//    int size = langs.size();
//    jobjectArray ret = env->NewObjectArray (size, strClass, NULL);
//    for (int i = 0; i < size; i++)
//	  env->SetObjectArrayElement (ret, i, env->NewStringUTF (langs[i].c_str()));
//
//    return ret;
//}
