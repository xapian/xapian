// C++ includes
#include <string>
#include <vector>
#include <iostream>

// Open Muscat includes
#include "om/om.h"

// JNI includes
#include "com_webtop_om_OmMatchOptions.h"
#include "utils.h"

/*
 * Class:     com_webtop_om_OmMatchOptions
 * Method:    createNativeObject
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_webtop_om_OmMatchOptions_createNativeObject
  (JNIEnv *env, jobject obj)
{
    return (jlong) new OmMatchOptions ();    
}

/*
 * Class:     com_webtop_om_OmMatchOptions
 * Method:    deleteNativeObject
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_webtop_om_OmMatchOptions_deleteNativeObject
  (JNIEnv *env, jobject obj)
{
    delete (OmMatchOptions*) tryGetLongField (env, obj, "nativePtr");    
}

/*
 * Class:     com_webtop_om_OmMatchOptions
 * Method:    set_collapse_key
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_webtop_om_OmMatchOptions_set_1collapse_1key
  (JNIEnv *env, jobject obj, jint key)
{
    OmMatchOptions* mopt = (OmMatchOptions*) tryGetLongField (env, obj, "nativePtr");
    mopt->set_collapse_key ((om_keyno) key);
}

/*
 * Class:     com_webtop_om_OmMatchOptions
 * Method:    set_no_collapse
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_webtop_om_OmMatchOptions_set_1no_1collapse
  (JNIEnv *env, jobject obj)
{
    OmMatchOptions* mopt = (OmMatchOptions*) tryGetLongField (env, obj, "nativePtr");
    mopt->set_no_collapse ();
}

/*
 * Class:     com_webtop_om_OmMatchOptions
 * Method:    set_sort_forward
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_com_webtop_om_OmMatchOptions_set_1sort_1forward
  (JNIEnv *env, jobject obj, jboolean forward)
{
    OmMatchOptions* mopt = (OmMatchOptions*) tryGetLongField (env, obj, "nativePtr");
    mopt->set_sort_forward ((bool) forward);
}

/*
 * Class:     com_webtop_om_OmMatchOptions
 * Method:    set_percentage_cutoff
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_webtop_om_OmMatchOptions_set_1percentage_1cutoff
  (JNIEnv *env, jobject obj, jint cutoff)
{
    OmMatchOptions* mopt = (OmMatchOptions*) tryGetLongField (env, obj, "nativePtr");
    mopt->set_percentage_cutoff ((int) cutoff);
}

/*
 * Class:     com_webtop_om_OmMatchOptions
 * Method:    set_max_or_terms
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_webtop_om_OmMatchOptions_set_1max_1or_1terms
  (JNIEnv *env, jobject obj, jint terms)
{
    OmMatchOptions* mopt = (OmMatchOptions*) tryGetLongField (env, obj, "nativePtr");
    mopt->set_max_or_terms ((om_termcount) terms);
}

