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
  (JNIEnv *, jobject);

/*
 * Class:     com_muscat_om_OmDocumentContents
 * Method:    deleteNativeObject
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmDocumentContents_deleteNativeObject
  (JNIEnv *, jobject);

/*
 * Class:     com_muscat_om_OmDocumentContents
 * Method:    set_data
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmDocumentContents_set_1data
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_muscat_om_OmDocumentContents
 * Method:    add_key
 * Signature: (ILjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmDocumentContents_add_1key
  (JNIEnv *, jobject, jint, jstring);

/*
 * Class:     com_muscat_om_OmDocumentContents
 * Method:    add_posting
 * Signature: (Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmDocumentContents_add_1posting
  (JNIEnv *, jobject, jstring, jint);

/*
 * Class:     com_muscat_om_OmDocumentContents
 * Method:    get_description
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_muscat_om_OmDocumentContents_get_1description
  (JNIEnv *, jobject);
