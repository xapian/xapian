// C++ includes
#include <string>

// Open Muscat includes
#include "om/om.h"
#include "om/omenquire.h"

// JNI includes
#include "com_webtop_om_OmPrefixExpandDecider.h"
#include "utils.h"

class OmPrefixExpandDecider : public OmExpandDecider {
    private:
    string prefix;
    int prelen;

    public:
    OmPrefixExpandDecider (const string& p) { 
	prefix = p;
	prelen = p.length();
    }

    int operator() (const om_termname & tname) const {
	return (tname.compare (prefix, 0, prelen) == 0);
    }
};


/*
 * Class:     com_webtop_om_OmPrefixExpandDecider
 * Method:    createNativeObject
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_com_webtop_om_OmPrefixExpandDecider_createNativeObject
  (JNIEnv *env, jobject obj, jstring prefix)
{
    return (jlong) new OmPrefixExpandDecider (getStringValue (env, prefix));
}

/*
 * Class:     com_webtop_om_OmPrefixExpandDecider
 * Method:    deleteNativeObject
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_webtop_om_OmPrefixExpandDecider_deleteNativeObject
  (JNIEnv *env, jobject obj)
{
    delete (OmPrefixExpandDecider*) tryGetLongField (env, obj, "nativePtr");
}
