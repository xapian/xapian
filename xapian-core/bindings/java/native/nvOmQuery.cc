// C++ includes
#include <string>
#include <vector>
#include <string.h>

// Open Muscat includes
#include "om/om.h"

// JNI includes
#include "com_muscat_om_OmQuery.h"
#include "utils.h"

/*
 * Class:     com_muscat_om_OmQuery
 * Method:    createNativeObject
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_muscat_om_OmQuery_createNativeObject__
  (JNIEnv *env, jobject obj)
{

    return (jlong) new OmQuery ();
}

/*
 * Class:     com_muscat_om_OmQuery
 * Method:    deleteNativeObject
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmQuery_deleteNativeObject
  (JNIEnv *env, jobject obj)
{
    delete (OmQuery*) tryGetLongField (env, obj, "nativePtr");
}


/*
 * Class:     com_muscat_om_OmQuery
 * Method:    createNativeObject
 * Signature: (Ljava/lang/String;II)J
 */
JNIEXPORT jlong JNICALL Java_com_muscat_om_OmQuery_createNativeObject__Ljava_lang_String_2II
  (JNIEnv *env, jobject obj, jstring term, jint count, jint pos)
{
    string thing = getStringValue (env, term);
    return (jlong) new OmQuery (thing,
				(om_termcount) count,
				(om_termpos) pos);
}

/* helper function */
om_queryop transOp (JNIEnv* env, jstring op) {
    char* op_n = (char*) env->GetStringUTFChars (op, NULL);
    om_queryop ret = OM_MOP_LEAF;
    if (! strcasecmp (op_n, "AND")) 
	ret = OM_MOP_AND;
    else if (! strcasecmp (op_n, "OR")) 
	ret = OM_MOP_OR;
    else if (! strcasecmp (op_n, "AND NOT")) 
	ret = OM_MOP_AND_NOT;
    else if (! strcasecmp (op_n, "XOR")) 
	ret = OM_MOP_XOR;
    else if (! strcasecmp (op_n, "AND MAYBE")) 
	ret = OM_MOP_AND_MAYBE;
    else if (! strcasecmp (op_n, "FILTER")) 
	ret = OM_MOP_FILTER;
	
    env->ReleaseStringUTFChars (op, op_n);

    if (ret == OM_MOP_LEAF)
	throwNewException (env, "java/lang/RuntimeException", "invalid query operator name");

    return ret;
}
    

/*
 * Class:     com_muscat_om_OmQuery
 * Method:    createNativeObject
 * Signature: (Ljava/lang/String;Lcom/muscat/om/OmQuery;Lcom/muscat/om/OmQuery;)J
 */
JNIEXPORT jlong JNICALL Java_com_muscat_om_OmQuery_createNativeObject__Ljava_lang_String_2Lcom_muscat_om_OmQuery_2Lcom_muscat_om_OmQuery_2
  (JNIEnv *env, jobject obj, jstring op, jobject left, jobject right)
{

    OmQuery* left_n  = (OmQuery*) tryGetLongField (env, left, "nativePtr");
    OmQuery* right_n = (OmQuery*) tryGetLongField (env, right, "nativePtr");
    om_queryop op_n = transOp (env, op);

    return (jlong) new OmQuery (op_n, *left_n, *right_n);
}

/*
 * Class:     com_muscat_om_OmQuery
 * Method:    createNativeObject
 * Signature: (Ljava/lang/String;[Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_com_muscat_om_OmQuery_createNativeObject__Ljava_lang_String_2_3Ljava_lang_String_2
  (JNIEnv *env, jobject obj, jstring op, jobjectArray terms)
{

    vector<om_termname> terms_n;
    int len = env->GetArrayLength (terms);
    for (int i = 0; i < len; i++) {
	jstring jterm = (jstring) env->GetObjectArrayElement (terms, i);
	terms_n.push_back (getStringValue (env, jterm));
	env->DeleteLocalRef (jterm);
    }
    
    om_queryop op_n = transOp (env, op);
    
    return (jlong) new OmQuery (op_n, terms_n.begin(), terms_n.end());
}

/*
 * Class:     com_muscat_om_OmQuery
 * Method:    get_terms
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_muscat_om_OmQuery_get_1terms
  (JNIEnv *env, jobject obj)
{
    om_termname_list terms;

    OmQuery* query = (OmQuery*) tryGetLongField (env, obj, "nativePtr");
    try {
	terms = query->get_terms();
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }

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
