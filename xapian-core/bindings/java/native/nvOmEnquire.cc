// C++ includes
#include <string>
#include <vector>
#include <iostream>

// Open Muscat includes
#include "om/om.h"

// JNI includes
#include "com_muscat_om_OmEnquire.h"
#include "utils.h"

/*
 * Class:     com_muscat_om_OmEnquire
 * Method:    createNativeObject
 * Signature: (Lcom/muscat/om/OmDatabaseGroup;)J
 */
JNIEXPORT jlong JNICALL Java_com_muscat_om_OmEnquire_createNativeObject
  (JNIEnv *env, jobject obj, jobject db)
{
    OmDatabaseGroup* db_n = (OmDatabaseGroup*) tryGetLongField (env, db, "nativePtr");
    jlong enq = 0;
    try {
	enq = (jlong) new OmEnquire (*db_n);
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }
    
    return enq;
}

/*
 * Class:     com_muscat_om_OmEnquire
 * Method:    deleteNativeObject
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmEnquire_deleteNativeObject
  (JNIEnv *env, jobject obj)
{
    delete (OmEnquire*) tryGetLongField (env, obj, "nativePtr");
}

/*
 * Class:     com_muscat_om_OmEnquire
 * Method:    get_doc
 * Signature: (Lcom/muscat/om/OmMSetItem;)Lcom/muscat/om/OmDocument;
 */
JNIEXPORT jobject JNICALL Java_com_muscat_om_OmEnquire_get_1doc__Lcom_muscat_om_OmMSetItem_2
  (JNIEnv *env, jobject obj, jobject mitem)
{
    OmEnquire* enq = (OmEnquire*) tryGetLongField (env, obj, "nativePtr");
    OmMSetItem* mitem_n = (OmMSetItem*) tryGetLongField (env, mitem, "nativePtr");

    OmDocument* doc_n = new OmDocument (enq->get_doc (*mitem_n));
    return makeReturnObject (env, "com/muscat/om/OmDocument", (jlong) doc_n);
}

/*
 * Class:     com_muscat_om_OmEnquire
 * Method:    get_doc
 * Signature: (I)Lcom/muscat/om/OmDocument;
 */
JNIEXPORT jobject JNICALL Java_com_muscat_om_OmEnquire_get_1doc__I
  (JNIEnv *env, jobject obj, jint did)
{
    OmEnquire* enq = (OmEnquire*) tryGetLongField (env, obj, "nativePtr");
    OmDocument* doc_n = new OmDocument (enq->get_doc ((om_docid) did));
    return makeReturnObject (env, "com/muscat/om/OmDocument", (jlong) doc_n);
}

/*
 * Class:     com_muscat_om_OmEnquire
 * Method:    get_mset
 * Signature: (JJLcom/muscat/om/OmRSet;Lcom/muscat/om/OmMatchOptions;Lcom/muscat/om/OmMatchDecider;)Lcom/muscat/om/OmMSet;
 */
JNIEXPORT jobject JNICALL Java_com_muscat_om_OmEnquire_get_1mset
  (JNIEnv *env, jobject obj, jint first, jint max, 
   jobject omrset, jobject omoptions, jobject ommatchdecider)
{
    OmEnquire* enq = (OmEnquire*) tryGetLongField (env, obj, "nativePtr");
    OmRSet*          rset = omrset ? 
	(OmRSet*)           tryGetLongField (env, omrset, "nativePtr") : NULL;
    OmMatchOptions*  mopt = omoptions ? 
	(OmMatchOptions*)   tryGetLongField (env, omoptions, "nativePtr") : NULL;
    OmMatchDecider*  mdec = ommatchdecider ? 
	(OmMatchDecider*)   tryGetLongField (env, ommatchdecider, "nativePtr") : NULL;

    try {
	OmMSet* mset = new OmMSet (enq->get_mset ((om_doccount) first, (om_doccount) max, 
						  rset, mopt, mdec));
	return makeReturnObject (env, "com/muscat/om/OmMSet", (jlong) mset);
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }
    return NULL;						 
}

/*
 * Class:     com_muscat_om_OmEnquire
 * Method:    set_query
 * Signature: (Lcom/muscat/om/OmQuery;)V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmEnquire_set_1query
  (JNIEnv *env, jobject obj, jobject query) 
{
    OmEnquire* enq = (OmEnquire*) tryGetLongField (env, obj, "nativePtr");
    OmQuery* query_n = (OmQuery*) tryGetLongField (env, query, "nativePtr");
    try {
	enq->set_query (*query_n);
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }    
}

/*
 * Class:     com_muscat_om_OmEnquire
 * Method:    get_eset
 * Signature: (ILcom/muscat/om/OmRSet;Lcom/muscat/om/OmExpandOptions;Lcom/muscat/om/OmExpandDecider;)Lcom/muscat/om/OmESet;
 */
JNIEXPORT jobject JNICALL Java_com_muscat_om_OmEnquire_get_1eset
  (JNIEnv *env, jobject obj, jint maxterms, jobject rset, jobject eoptions, jobject edecider)
{
    OmEnquire* enq              = (OmEnquire*)       tryGetLongField (env, obj, "nativePtr");
    OmRSet* rset_n              = (OmRSet*)          tryGetLongField (env, rset, "nativePtr");
    
    OmExpandOptions* eoptions_n = (eoptions == NULL) ? 0 :
	(OmExpandOptions*) tryGetLongField (env, eoptions, "nativePtr");

    OmExpandDecider* edecider_n = (edecider == NULL) ? 0 :
	(OmExpandDecider*) tryGetLongField (env, edecider, "nativePtr");

    try {
	OmESet* ret = new OmESet (enq->get_eset ((om_termcount) maxterms, *rset_n, 
						 eoptions_n, edecider_n));
	
	return makeReturnObject (env, "com/muscat/om/OmESet", (jlong) ret);
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }

    return NULL;
}


/*
 * Class:     com_muscat_om_OmEnquire
 * Method:    get_matching_terms
 * Signature: (I)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_muscat_om_OmEnquire_get_1matching_1terms
  (JNIEnv *env, jobject obj, jint did) 
{
    OmEnquire* enq = (OmEnquire*) tryGetLongField (env, obj, "nativePtr");
    om_termname_list terms = enq->get_matching_terms ((om_docid) did);
    int termcount = terms.size();
    jclass strclass = env->FindClass ("java/lang/String");
    jobjectArray ret = env->NewObjectArray (termcount, strclass, NULL);
    int p = 0;
    for (om_termname_list::iterator it = terms.begin ();
	 it != terms.end (); it++, p++)
    {
	env->SetObjectArrayElement (ret, p, env->NewStringUTF (((string) *it).c_str()));
    }	

    return ret;
}

