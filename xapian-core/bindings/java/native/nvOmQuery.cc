// C++ includes
#include <string>
#include <vector>
#include <cstring>

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
    try {
	return (jlong) new OmQuery (thing,
				    (om_termcount) count,
				    (om_termpos) pos);
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }
    return 0;
}

/* helper function */
OmQuery::op transOp (JNIEnv* env, jstring op) {
    char* op_n = (char*) env->GetStringUTFChars (op, NULL);
    OmQuery::op ret = OmQuery::OP_LEAF;
    if (! strcasecmp (op_n, "AND"))
	ret = OmQuery::OP_AND;
    else if (! strcasecmp (op_n, "OR"))
	ret = OmQuery::OP_OR;
    else if (! strcasecmp (op_n, "AND NOT"))
	ret = OmQuery::OP_AND_NOT;
    else if (! strcasecmp (op_n, "XOR"))
	ret = OmQuery::OP_XOR;
    else if (! strcasecmp (op_n, "AND MAYBE"))
	ret = OmQuery::OP_AND_MAYBE;
    else if (! strcasecmp (op_n, "FILTER"))
	ret = OmQuery::OP_FILTER;
    else if (! strcasecmp (op_n, "NEAR"))
	ret = OmQuery::OP_NEAR;
    else if (! strcasecmp (op_n, "PHRASE"))
	ret = OmQuery::OP_PHRASE;

    env->ReleaseStringUTFChars (op, op_n);

    if (ret == OmQuery::OP_LEAF)
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
    OmQuery::op op_n = transOp (env, op);

    try {
	return (jlong) new OmQuery (op_n, *left_n, *right_n);
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }
    return 0;
}

/*
 * Class:     com_muscat_om_OmQuery
 * Method:    createNativeObject
 * Signature: (Ljava/lang/String;[Ljava/lang/String;I)J
 */
JNIEXPORT jlong JNICALL Java_com_muscat_om_OmQuery_createNativeObject__Ljava_lang_String_2_3Ljava_lang_String_2I
  (JNIEnv *env, jobject obj, jstring op, jobjectArray terms, jint window)
{

    vector<om_termname> terms_n;
    int len = env->GetArrayLength (terms);
    for (int i = 0; i < len; i++) {
	jstring jterm = (jstring) env->GetObjectArrayElement (terms, i);
	terms_n.push_back (getStringValue (env, jterm));
	env->DeleteLocalRef (jterm);
    }

    OmQuery::op op_n = transOp (env, op);

    return (jlong) new OmQuery (op_n, terms_n.begin(),
				terms_n.end(), (om_termpos)window);
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

/*
 * Class:     com_muscat_om_OmQuery
 * Method:    get_description
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_muscat_om_OmQuery_get_1description
  (JNIEnv *env, jobject obj)
{
    OmQuery* query = (OmQuery*) tryGetLongField (env, obj, "nativePtr");
    try {
	return env->NewStringUTF (query->get_description().c_str());
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }
    return NULL;
}

/*
 * Class:     com_muscat_om_OmQuery
 * Method:    get_length
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_muscat_om_OmQuery_get_1length
  (JNIEnv *env, jobject obj)
{
    OmQuery* query = (OmQuery*) tryGetLongField (env, obj, "nativePtr");
    try {
	return query->get_length();
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }
    return 0;
}

/*
 * Class:     com_muscat_om_OmQuery
 * Method:    set_bool
 * Signature: (Z)Z
 */
JNIEXPORT jboolean JNICALL Java_com_muscat_om_OmQuery_set_1bool
  (JNIEnv *env, jobject obj, jboolean isbool_)
{
    OmQuery* query = (OmQuery*) tryGetLongField (env, obj, "nativePtr");
    try {
	return (jboolean)query->set_bool((bool)isbool_);
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }
    return 0;
}


