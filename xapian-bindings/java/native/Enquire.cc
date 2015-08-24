/**
 Copyright (c) 2003, Technology Concepts & Design, Inc.
 Copyright (c) 2006,2008,2011, Olly Betts
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are permitted
 provided that the following conditions are met:

 - Redistributions of source code must retain the above copyright notice, this list of conditions
 and the following disclaimer.

 - Redistributions in binary form must reproduce the above copyright notice, this list of conditions
 and the following disclaimer in the documentation and/or other materials provided with the distribution.

 - Neither the name of Technology Concepts & Design, Inc. nor the names of its contributors may be used to
 endorse or promote products derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.
 **/
 
#include "xapian_jni.h"

using namespace std;
using namespace Xapian;

class JavaMatchDecider : public Xapian::MatchDecider {
    private:
        JNIEnv *env;
        jclass clazz;

        jobject _obj;
        jclass _deciderclass;
        jmethodID _acceptmethod;
    public:
        JavaMatchDecider(JNIEnv *xenv, jclass xclazz, jobject &obj) : MatchDecider() {
            env = xenv;
            clazz = xclazz;
            _obj = obj;
            TRY
                _deciderclass = env->GetObjectClass(obj);
                if (!_deciderclass) goto on_error;

                _acceptmethod = env->GetMethodID(_deciderclass, "accept", "(Lorg/xapian/Document;)Z");
                if (!_acceptmethod) goto on_error;

on_error:
            CATCH(;)
        }

    bool operator() (const Xapian::Document &doc) const {
        TRY
            jmethodID ctorid;
            jobject document;
            long docid = id_from_obj(new Document(doc));

            jclass documentclass = env->FindClass("org/xapian/Document");
            if (!documentclass) goto on_error;

            ctorid = env->GetMethodID(documentclass, "<init>", "(J)V");
            if (!ctorid) goto on_error;

            document = env->NewObject(documentclass, ctorid, docid);
            if (!document) goto on_error;

            return env->CallBooleanMethod(_obj, _acceptmethod, document);

on_error:       // I can also "program" in VisualBasic
        CATCH(-1)
    }
};

class JavaExpandDecider : public Xapian::ExpandDecider {
    private:
        JNIEnv *env;
        jclass clazz;

        jobject _obj;
        jclass _deciderclass;
        jmethodID _acceptmethod;
    public:
        JavaExpandDecider(JNIEnv *xenv, jclass xclazz, jobject &obj) : ExpandDecider() {
            env = xenv;
            clazz = xclazz;
            _obj = obj;
            TRY
                _deciderclass = env->GetObjectClass(obj);
                if (!_deciderclass) goto on_error;

                _acceptmethod = env->GetMethodID(_deciderclass, "accept", "(Ljava/lang/String;)Z");
                if (!_acceptmethod) goto on_error;

on_error:
            CATCH(;)
        }

    bool operator() (const std::string &tname) const {
        TRY
            return env->CallBooleanMethod(_obj, _acceptmethod, env->NewStringUTF(tname.c_str()));
        CATCH(-1)
    }
};


JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_enquire_1new (JNIEnv *env, jclass clazz, jlong dbid) {
    TRY
        Database *db = obj_from_id<Database *>(dbid);
        Enquire *e = new Enquire(*db);
        return id_from_obj(e);
    CATCH(-1)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_enquire_1set_1query (JNIEnv *env, jclass clazz, jlong eid, jlong qid) {
    TRY
        Enquire *e = obj_from_id<Enquire *>(eid);
        Query *q = obj_from_id<Query *>(qid);
        e->set_query(*q);
    CATCH(;)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_enquire_1set_1query (JNIEnv *env, jclass clazz, jlong eid, jlong qid, jint qlen) {
    TRY
        Enquire *e = obj_from_id<Enquire *>(eid);
        Query *q = obj_from_id<Query *>(qid);
        e->set_query(*q, qlen);
    CATCH(;)
}

// We don't need this wrapped by the JNI - we just handle it at the Java level
// instead.
#if 0
JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_enquire_1get_1query (JNIEnv *env, jclass clazz, jlong eid) {
    TRY
        Enquire *e = obj_from_id<Enquire *>(eid);
        Query q = e->get_query();
	// Insert magic here!
        return q.getMyID();
    CATCH(-1)
}
#endif

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_enquire_1set_1collapse_1key (JNIEnv *env, jclass clazz, jlong eid, jlong collapse_key) {
    TRY
        Enquire *e = obj_from_id<Enquire *>(eid);
        e->set_collapse_key(collapse_key);
    CATCH(;)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_enquire_1set_1sort_1forward (JNIEnv *env, jclass clazz, jlong eid, jboolean forward) {
    TRY
        Enquire *e = obj_from_id<Enquire *>(eid);
        e->set_docid_order(forward ? Xapian::Enquire::ASCENDING : Xapian::Enquire::DESCENDING);
    CATCH(;)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_enquire_1set_1cutoff (JNIEnv *env, jclass clazz, jlong eid, jint percent_cutoff, jdouble cutoff) {
    TRY
        Enquire *e = obj_from_id<Enquire *>(eid);
        e->set_cutoff(percent_cutoff, cutoff);
    CATCH(;)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_enquire_1set_1sorting (JNIEnv *env, jclass clazz, jlong eid, jlong sortkey, jint bands) {
    TRY
        Enquire *e = obj_from_id<Enquire *>(eid);
        if (bands) {
            e->set_sort_by_value(sortkey);
        } else {
            e->set_sort_by_relevance();
        }
    CATCH(;)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_enquire_1get_1mset (JNIEnv *env, jclass clazz, jlong eid, jlong first, jlong maxitems, jlong rsetid, jobject md) {
    TRY
        Enquire *e = obj_from_id<Enquire *>(eid);
        RSet *rset;
	MatchDecider *mdecider;
        MSet *mset;

	rset = rsetid ? obj_from_id<RSet *>(rsetid) : NULL;

	if (md) {
	    JavaMatchDecider mdecider(env, clazz, md);
	    mset = new MSet (e->get_mset(first, maxitems, rset, &mdecider));
	} else {
	    mset = new MSet (e->get_mset(first, maxitems, rset));
	}

        return id_from_obj(mset);
    CATCH(-1)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_enquire_1get_1eset__JJJIDLorg_xapian_ExpandDecider_2 (JNIEnv *env, jclass clazz, jlong eid, jlong maxitems, jlong rsetid, jint flags, jdouble k, jobject ed) {
    TRY
        Enquire *e = obj_from_id<Enquire *>(eid);
        RSet *rset = obj_from_id<RSet *>(rsetid);
        ESet *eset = new ESet (e->get_eset(maxitems, *rset, flags, k, ed ? new JavaExpandDecider(env, clazz, ed) : NULL));
        return id_from_obj(eset);
    CATCH(-1)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_enquire_1get_1eset__JJJLorg_xapian_ExpandDecider_2 (JNIEnv *env, jclass clazz, jlong eid, jlong maxitems, jlong rsetid, jobject ed) {
    TRY
        Enquire *e = obj_from_id<Enquire *>(eid);
        RSet *rset = obj_from_id<RSet *>(rsetid);
        ESet *eset = new ESet(e->get_eset(maxitems, *rset, ed ? new JavaExpandDecider(env, clazz, ed) : NULL));
        return id_from_obj(eset);
    CATCH(-1)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_enquire_1get_1matching_1terms_1begin (JNIEnv *env, jclass clazz, jlong eid, jlong dbdocid) {
    TRY
        Enquire *e = obj_from_id<Enquire *>(eid);
        TermIterator *itr = new TermIterator (e->get_matching_terms_begin(dbdocid));
        return id_from_obj(itr);
    CATCH(-1)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_enquire_1get_1matching_1terms_1end (JNIEnv *env, jclass clazz, jlong eid, jlong dbdocid) {
    TRY
        Enquire *e = obj_from_id<Enquire *>(eid);
        TermIterator *itr = new TermIterator (e->get_matching_terms_end(dbdocid));
        return id_from_obj(itr);
    CATCH(-1)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_enquire_1get_1matching_1terms_1begin_1by_1msetiterator (JNIEnv *env, jclass clazz, jlong eid, jlong msetiteratorid) {
    TRY
        Enquire *e = obj_from_id<Enquire *>(eid);
        MSetIterator *msetitr = obj_from_id<MSetIterator *>(msetiteratorid);
        TermIterator *itr = new TermIterator (e->get_matching_terms_begin(*msetitr));
        return id_from_obj(itr);
    CATCH(-1)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_enquire_1get_1matching_1terms_1end_1by_1msetiterator (JNIEnv *env, jclass clazz, jlong eid, jlong msetiteratorid) {
    TRY
        Enquire *e = obj_from_id<Enquire *>(eid);
        MSetIterator *msetitr = obj_from_id<MSetIterator *>(msetiteratorid);
        TermIterator *itr = new TermIterator (e->get_matching_terms_end(*msetitr));
        return id_from_obj(itr);
    CATCH(-1)
}

JNIEXPORT jstring JNICALL Java_org_xapian_XapianJNI_enquire_1get_1description (JNIEnv *env, jclass clazz, jlong eid) {
    TRY
        Enquire *e = obj_from_id<Enquire *>(eid);
        return env->NewStringUTF(e->get_description().c_str());
    CATCH(NULL)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_enquire_1finalize (JNIEnv *env, jclass clazz, jlong eid) {
    delete obj_from_id<Enquire *>(eid);
}
