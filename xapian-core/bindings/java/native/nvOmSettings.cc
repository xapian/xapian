/* nvOmSettings.cc: "global" settings object
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include <om/om.h>

#include "com_muscat_om_OmSettings.h"
#include "utils.h"
/*
 * Class:     com_muscat_om_OmSettings
 * Method:    createNativeObject
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_muscat_om_OmSettings_createNativeObject
  (JNIEnv *env, jobject obj)
{
    jlong enq = 0;
    try {
	enq = (jlong) new OmSettings ();
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }

    return enq;
}

/*
 * Class:     com_muscat_om_OmSettings
 * Method:    deleteNativeObject
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmSettings_deleteNativeObject
  (JNIEnv *env, jobject obj)
{
    delete (OmSettings*) tryGetLongField (env, obj, "nativePtr");
}

/*
 * Class:     com_muscat_om_OmSettings
 * Method:    set
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmSettings_set__Ljava_lang_String_2Ljava_lang_String_2
  (JNIEnv *env, jobject obj, jstring key, jstring value)
{
    try {
	std::string key_n(getStringValue(env, key));
	std::string value_n(getStringValue(env, value));

	OmSettings *obj_n = (OmSettings *)tryGetLongField(env, obj, "nativePtr");
	obj_n->set(key_n, value_n);
    } catch (OmError &err) {
	handleNativeError (env, err);
    }
}

/*
 * Class:     com_muscat_om_OmSettings
 * Method:    set
 * Signature: (Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmSettings_set__Ljava_lang_String_2I
  (JNIEnv *env, jobject obj, jstring key, jint value)
{
    try {
	std::string key_n(getStringValue(env, key));

	OmSettings *obj_n = (OmSettings *)tryGetLongField(env, obj, "nativePtr");
	obj_n->set(key_n, (int) value);
    } catch (OmError &err) {
	handleNativeError (env, err);
    }
}

/*
 * Class:     com_muscat_om_OmSettings
 * Method:    set
 * Signature: (Ljava/lang/String;D)V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmSettings_set__Ljava_lang_String_2D
  (JNIEnv *env, jobject obj, jstring key, jdouble value)
{
    try {
	std::string key_n(getStringValue(env, key));

	OmSettings *obj_n = (OmSettings *)tryGetLongField(env, obj, "nativePtr");
	obj_n->set(key_n, (double) value);
    } catch (OmError &err) {
	handleNativeError (env, err);
    }
}

/*
 * Class:     com_muscat_om_OmSettings
 * Method:    set
 * Signature: (Ljava/lang/String;Z)V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmSettings_set__Ljava_lang_String_2Z
  (JNIEnv *env, jobject obj, jstring key, jboolean value)
{
    try {
	std::string key_n(getStringValue(env, key));

	OmSettings *obj_n = (OmSettings *)tryGetLongField(env, obj, "nativePtr");
	obj_n->set(key_n, (bool) value);
    } catch (OmError &err) {
	handleNativeError (env, err);
    }
}

/*
 * Class:     com_muscat_om_OmSettings
 * Method:    set
 * Signature: (Ljava/lang/String;[Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_muscat_om_OmSettings_set__Ljava_lang_String_2_3Ljava_lang_String_2
  (JNIEnv *env, jobject obj, jstring key, jobjectArray value)
{
    try {
	std::string key_n(getStringValue(env, key));

	OmSettings *obj_n = (OmSettings *)tryGetLongField(env, obj, "nativePtr");

	std::vector<std::string> value_n;

	int len = env->GetArrayLength (value);
	for (int i=0; i<len; ++i) {
	    jstring jterm = (jstring) env->GetObjectArrayElement (value, i);
	    value_n.push_back(getStringValue(env, jterm));
	    env->DeleteLocalRef (jterm);
	}
	obj_n->set(key_n, value_n.begin(), value_n.end());
    } catch (OmError &err) {
	handleNativeError (env, err);
    }
}

/*
 * Class:     com_muscat_om_OmSettings
 * Method:    get
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_muscat_om_OmSettings_get__Ljava_lang_String_2
  (JNIEnv *env, jobject obj, jstring key)
{
    OmSettings *param = (OmSettings *)tryGetLongField(env, obj, "nativePtr");
    try {
	return env->NewStringUTF(param->get(getStringValue(env, key)).c_str());
    } catch (OmError &err) {
	handleNativeError (env, err);
    }
    return NULL;
}

/*
 * Class:     com_muscat_om_OmSettings
 * Method:    get
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_muscat_om_OmSettings_get__Ljava_lang_String_2Ljava_lang_String_2
  (JNIEnv *env, jobject obj, jstring key, jstring def)
{
    OmSettings *param = (OmSettings *)tryGetLongField(env, obj, "nativePtr");
    try {
	return env->NewStringUTF(param->get(getStringValue(env, key),
					    getStringValue(env, def)).c_str());
    } catch (OmError &err) {
	handleNativeError (env, err);
    }
    return NULL;
}

/*
 * Class:     com_muscat_om_OmSettings
 * Method:    get_int
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_muscat_om_OmSettings_get_1int__Ljava_lang_String_2
  (JNIEnv *env, jobject obj, jstring key)
{
    OmSettings *param = (OmSettings *)tryGetLongField(env, obj, "nativePtr");
    try {
	return (jint)param->get_int(getStringValue(env, key));
    } catch (OmError &err) {
	handleNativeError (env, err);
    }
    return NULL;
}

/*
 * Class:     com_muscat_om_OmSettings
 * Method:    get_int
 * Signature: (Ljava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_com_muscat_om_OmSettings_get_1int__Ljava_lang_String_2I
  (JNIEnv *env, jobject obj, jstring key, jint def)
{
    OmSettings *param = (OmSettings *)tryGetLongField(env, obj, "nativePtr");
    try {
	return (jint)param->get_int(getStringValue(env, key),
				(int) def);
    } catch (OmError &err) {
	handleNativeError (env, err);
    }
    return NULL;
}

/*
 * Class:     com_muscat_om_OmSettings
 * Method:    get_bool
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_muscat_om_OmSettings_get_1bool__Ljava_lang_String_2
  (JNIEnv *env, jobject obj, jstring key)
{
    OmSettings *param = (OmSettings *)tryGetLongField(env, obj, "nativePtr");
    try {
	return (jboolean)param->get_int(getStringValue(env, key));
    } catch (OmError &err) {
	handleNativeError (env, err);
    }
    return NULL;
}

/*
 * Class:     com_muscat_om_OmSettings
 * Method:    get_bool
 * Signature: (Ljava/lang/String;Z)Z
 */
JNIEXPORT jboolean JNICALL Java_com_muscat_om_OmSettings_get_1bool__Ljava_lang_String_2Z
  (JNIEnv *env, jobject obj, jstring key, jboolean def)
{
    OmSettings *param = (OmSettings *)tryGetLongField(env, obj, "nativePtr");
    try {
	return (jboolean)param->get_int(getStringValue(env, key),
				(bool) def);
    } catch (OmError &err) {
	handleNativeError (env, err);
    }
    return NULL;
}

/*
 * Class:     com_muscat_om_OmSettings
 * Method:    get_real
 * Signature: (Ljava/lang/String;)D
 */
JNIEXPORT jdouble JNICALL Java_com_muscat_om_OmSettings_get_1real__Ljava_lang_String_2
  (JNIEnv *env, jobject obj, jstring key)
{
    OmSettings *param = (OmSettings *)tryGetLongField(env, obj, "nativePtr");
    try {
	return (jdouble)param->get_real(getStringValue(env, key));
    } catch (OmError &err) {
	handleNativeError (env, err);
    }
    return NULL;
}

/*
 * Class:     com_muscat_om_OmSettings
 * Method:    get_real
 * Signature: (Ljava/lang/String;D)D
 */
JNIEXPORT jdouble JNICALL Java_com_muscat_om_OmSettings_get_1real__Ljava_lang_String_2D
  (JNIEnv *env, jobject obj, jstring key, jdouble def)
{
    OmSettings *param = (OmSettings *)tryGetLongField(env, obj, "nativePtr");
    try {
	return (jdouble)param->get_int(getStringValue(env, key),
				(double) def);
    } catch (OmError &err) {
	handleNativeError (env, err);
    }
    return NULL;
}

/*
 * Class:     com_muscat_om_OmSettings
 * Method:    get_vector
 * Signature: (Ljava/lang/String;)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_muscat_om_OmSettings_get_1vector
  (JNIEnv *env, jobject obj, jstring key)
{
    std::vector<std::string> value_n;

    OmSettings* param = (OmSettings*) tryGetLongField (env, obj, "nativePtr");
    try {
	value_n = param->get_vector(getStringValue(env, key));
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }

    int len = value_n.size();
    jclass strclass = env->FindClass ("java/lang/String");
    jobjectArray ret = env->NewObjectArray (len, strclass, NULL);
    int p = 0;
    for (std::vector<std::string>::const_iterator it = value_n.begin ();
	 it != value_n.end (); it++, p++)
    {
	env->SetObjectArrayElement (ret, p, env->NewStringUTF (((string) *it).c_str()));
    }

    return ret;
}

/*
 * Class:     com_muscat_om_OmSettings
 * Method:    get_description
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_muscat_om_OmSettings_get_1description
  (JNIEnv *env, jobject obj)
{
    OmSettings* param = (OmSettings*) tryGetLongField (env, obj, "nativePtr");
    try {
	return env->NewStringUTF (param->get_description().c_str());
    }
    catch (OmError& err) {
	handleNativeError (env, err);
    }
    return NULL;
}

