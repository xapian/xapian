// Open Muscat includes
#include "om/om.h"

// JNI includes
#include "com_muscat_om_OmDocumentContents.h"
#include "utils.h"

/*
 * Class:     com_muscat_om_OmDocumentContents
 * Method:    createNativeObject
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_muscat_om_OmDocumentContents_createNativeObject
  (JNIEnv *env, jobject obj)
{
    return (jlong) new OmDocumentContents();
}

/*
 * Class:     com_muscat_om_OmDocumentContents
 * Method:    deleteNativeObject
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmDocumentContents_deleteNativeObject
  (JNIEnv *env, jobject obj)
{
    delete (OmDocumentContents *) tryGetLongField(env, obj, "nativePtr");
}

/*
 * Class:     com_muscat_om_OmDocumentContents
 * Method:    set_data
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmDocumentContents_set_1data
  (JNIEnv *env, jobject obj, jstring data)
{
    OmDocumentContents *contents = (OmDocumentContents *)tryGetLongField(env, obj, "nativePtr");

    std::string data_n = getStringValue(env, data);

    contents->data.value = data_n;
}

/*
 * Class:     com_muscat_om_OmDocumentContents
 * Method:    add_key
 * Signature: (ILjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmDocumentContents_add_1key
  (JNIEnv *env, jobject obj, jint keyno, jstring value)
{
    OmDocumentContents *contents = (OmDocumentContents *)tryGetLongField(env, obj, "nativePtr");

    std::string value_n = getStringValue(env, value);

    contents->keys[(om_keyno)keyno] = value_n;
}

/*
 * Class:     com_muscat_om_OmDocumentContents
 * Method:    add_posting
 * Signature: (Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmDocumentContents_add_1posting
  (JNIEnv *env, jobject obj, jstring tname, jint tpos)
{
    OmDocumentContents *contents = (OmDocumentContents *)tryGetLongField(env, obj, "nativePtr");

    std::string tname_n = getStringValue(env, tname);

    contents->add_posting(tname_n, (om_termpos)tpos);
}

/*
 * Class:     com_muscat_om_OmDocumentContents
 * Method:    get_description
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_muscat_om_OmDocumentContents_get_1description
  (JNIEnv *env, jobject obj)
{
    OmDocumentContents* contents = (OmDocumentContents*) tryGetLongField (env, obj, "nativePtr");
    try {
	return env->NewStringUTF (contents->get_description().c_str());
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }
    return NULL;
}
