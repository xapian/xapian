// C++ includes
#include <string>

// Open Muscat includes
#include "om/om.h"
#include "om/omenquire.h"

// JNI includes
#include "com_webtop_om_OmDatabase.h"
#include "utils.h"

/*
 * Class:     com_webtop_om_OmPrefixExpandDecider
 * Method:    createNativeObject
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_com_webtop_om_OmPrefixExpandDecider_createNativeObject
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_webtop_om_OmPrefixExpandDecider
 * Method:    deleteNativeObject
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_webtop_om_OmPrefixExpandDecider_deleteNativeObject
  (JNIEnv *, jobject);
