// C++ includes
#include <string>
#include <set>
#include <iostream>

// Open Muscat includes
#include "om/om.h"
#include "om/omenquire.h"

// JNI includes
#include "com_muscat_om_OmRSet.h"
#include "utils.h"

/*
 * Class:     com_muscat_om_OmRSet
 * Method:    createNativeObject
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_muscat_om_OmRSet_createNativeObject
  (JNIEnv *env, jobject obj)
{
    return (jlong) new OmRSet ();
}

/*
 * Class:     com_muscat_om_OmRSet
 * Method:    deleteNativeObject
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmRSet_deleteNativeObject
  (JNIEnv *env, jobject obj)
{
    delete (OmRSet*) tryGetLongField (env, obj, "nativePtr");
}

/*
 * Class:     com_muscat_om_OmRSet
 * Method:    get_items
 * Signature: ()[I
 */
JNIEXPORT jintArray JNICALL Java_com_muscat_om_OmRSet_get_1items
  (JNIEnv *env, jobject obj)
{
    OmRSet* rset = (OmRSet*) tryGetLongField (env, obj, "nativePtr");
    int size = rset->items.size();
    jintArray ret = env->NewIntArray (size);
    int p = 0;
    jint buf [size];
    for (set<om_docid>::iterator it = rset->items.begin();
	 it != rset->items.end();
	 it++)
    {
	buf[p++] = *it;
    }
    
    env->SetIntArrayRegion (ret, 0, size, buf);
    return ret;
}

/*
 * Class:     com_muscat_om_OmRSet
 * Method:    add_document
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmRSet_add_1document
  (JNIEnv *env, jobject obj, jint did)
{
    OmRSet* rset = (OmRSet*) tryGetLongField (env, obj, "nativePtr");
    try {
	rset->add_document ((om_docid) did);
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }    
}

/*
 * Class:     com_muscat_om_OmRSet
 * Method:    remove_document
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmRSet_remove_1document
  (JNIEnv *env, jobject obj, jint did)
{
    OmRSet* rset = (OmRSet*) tryGetLongField (env, obj, "nativePtr");
    try {
	rset->remove_document ((om_docid) did);
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }    
}
