/**
 Copyright (c) 2003, Technology Concepts & Design, Inc.
 Copyright (c) 2008,2011, Olly Betts
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

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_writabledatabase_1new__ (JNIEnv *env, jclass clazz) {
    TRY
        WritableDatabase *db = new WritableDatabase();
        return id_from_obj(db);
    CATCH(-1)
}


JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_inmemory_1open (JNIEnv *, jclass) {
    WritableDatabase *db = new WritableDatabase(InMemory::open());
    return id_from_obj(db);
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_writabledatabase_1new__Ljava_lang_String_2I(JNIEnv *env, jclass clazz, jstring path, jint mode) {
    TRY
        return id_from_obj(new WritableDatabase(cpp_string(env, path), mode));
    CATCH(-1)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_writabledatabase_1flush (JNIEnv *env, jclass clazz, jlong dbid) {
    TRY
        WritableDatabase *db = (WritableDatabase *) obj_from_id<Database *>(dbid);
        db->flush();
    CATCH(;)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_writabledatabase_1begin_1transaction (JNIEnv *env, jclass clazz, jlong dbid) {
    TRY
        WritableDatabase *db = (WritableDatabase *) obj_from_id<Database *>(dbid);
        db->begin_transaction();
    CATCH(;)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_writabledatabase_1commit_1transaction (JNIEnv *env, jclass clazz, jlong dbid) {
    TRY
        WritableDatabase *db = (WritableDatabase *) obj_from_id<Database *>(dbid);
        db->commit_transaction();
    CATCH(;)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_writabledatabase_1cancel_1transaction (JNIEnv *env, jclass clazz, jlong dbid) {
    TRY
        WritableDatabase *db = (WritableDatabase *) obj_from_id<Database *>(dbid);
        db->cancel_transaction();
    CATCH(;)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_writabledatabase_1add_1document (JNIEnv *env, jclass clazz, jlong dbid, jlong docid) {
    TRY
        WritableDatabase *db = (WritableDatabase *) obj_from_id<Database *>(dbid);
        Document *doc = obj_from_id<Document *>(docid);
        long id = db->add_document(*doc);
        return id;
    CATCH(-1)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_writabledatabase_1delete_1document (JNIEnv *env, jclass clazz, jlong dbid, jlong assigned_docid) {
    TRY
        WritableDatabase *db = (WritableDatabase *) obj_from_id<Database *>(dbid);
        db->delete_document(assigned_docid);
    CATCH(;)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_writabledatabase_1replace_1document (JNIEnv *env, jclass clazz, jlong dbid, jlong assigned_docid, jlong docid) {
    TRY
        WritableDatabase *db = (WritableDatabase *) obj_from_id<Database *>(dbid);
        Document *doc = obj_from_id<Document *>(docid);
        db->replace_document(assigned_docid, *doc);
    CATCH(;)
}

JNIEXPORT jstring JNICALL Java_org_xapian_XapianJNI_writabledatabase_1get_1description (JNIEnv *env, jclass clazz, jlong dbid) {
    TRY
        WritableDatabase *db = (WritableDatabase *) obj_from_id<Database *>(dbid);
        return env->NewStringUTF(db->get_description().c_str());
    CATCH(NULL)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_writabledatabase_1finalize (JNIEnv *env, jclass clazz, jlong dbid) {
    WritableDatabase *db = (WritableDatabase *) obj_from_id<Database *>(dbid);
    delete db;
}
