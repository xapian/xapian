// Xapian includes
#include <config.h>
#include "om/om.h"

// JNI includes
#include "com_muscat_om_OmDocument.h"
#include "utils.h"

/*
 * Class:     com_muscat_om_OmDocument
 * Method:    createNativeObject
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_muscat_om_OmDocument_createNativeObject
  (JNIEnv *env, jobject obj)
{
    return (jlong) new OmDocument();
}

/*
 * Class:     com_muscat_om_OmDocument
 * Method:    deleteNativeObject
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmDocument_deleteNativeObject
  (JNIEnv *env, jobject obj)
{
    delete (OmDocument *) tryGetLongField(env, obj, "nativePtr");
}

/*
 * Class:     com_muscat_om_OmDocument
 * Method:    set_data
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmDocument_set_1data
  (JNIEnv *env, jobject obj, jstring data)
{
    OmDocument *contents = (OmDocument *)tryGetLongField(env, obj, "nativePtr");

    std::string data_n = getStringValue(env, data);

    contents->data.value = data_n;
}

/*
 * Class:     com_muscat_om_OmDocument
 * Method:    add_key
 * Signature: (ILjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmDocument_add_1key
  (JNIEnv *env, jobject obj, jint keyno, jstring value)
{
    OmDocument *contents = (OmDocument *)tryGetLongField(env, obj, "nativePtr");

    std::string value_n = getStringValue(env, value);

    contents->keys[(om_keyno)keyno] = value_n;
}

/*
 * Class:     com_muscat_om_OmDocument
 * Method:    add_posting
 * Signature: (Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmDocument_add_1posting
  (JNIEnv *env, jobject obj, jstring tname, jint tpos)
{
    OmDocument *contents = (OmDocument *)tryGetLongField(env, obj, "nativePtr");

    std::string tname_n = getStringValue(env, tname);

    contents->add_posting(tname_n, (om_termpos)tpos);
}

/*
 * Class:     com_muscat_om_OmDocument
 * Method:    get_description
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_muscat_om_OmDocument_get_1description
  (JNIEnv *env, jobject obj)
{
    OmDocument* contents = (OmDocument*) tryGetLongField (env, obj, "nativePtr");
    try {
	return env->NewStringUTF (contents->get_description().c_str());
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }
    return NULL;
}
