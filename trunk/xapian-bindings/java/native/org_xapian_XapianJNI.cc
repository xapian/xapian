/**
 Copyright (c) 2003, Technology Concepts & Design, Inc.
 Copyright (c) 2006, Olly Betts
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
#include "org_xapian_XapianJNI.h"

using namespace Xapian;
using namespace std;

//
// define the things that were declared as 'extern'al
//

XapianObjectHolder<void *> *_database;    // for Xapian::Database *and* Xapian::WritableDatabase
XapianObjectHolder<Document *> *_document;
XapianObjectHolder<Enquire *> *_enquire;
XapianObjectHolder<ESet *> *_eset;
XapianObjectHolder<ESetIterator *> *_esetiterator;
XapianObjectHolder<MSet *> *_mset;
XapianObjectHolder<MSetIterator *> *_msetiterator;
XapianObjectHolder<PositionIterator *> *_positioniterator;
XapianObjectHolder<Query *> *_query;
XapianObjectHolder<RSet *> *_rset;
XapianObjectHolder<Stem *> *_stem;
XapianObjectHolder<TermIterator *> *_termiterator;

/**
 * Optional JNI function that java calls when it loads this library.
 */
jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    _database = new XapianObjectHolder <void *> ();    // for Xapian::Database *and* Xapian::WritableDatabase
    _document = new XapianObjectHolder <Document *> ();
    _enquire = new XapianObjectHolder <Enquire *> ();
    _eset = new XapianObjectHolder <ESet *> ();
    _esetiterator = new XapianObjectHolder <ESetIterator *> ();
    _mset = new XapianObjectHolder <MSet *> ();
    _msetiterator = new XapianObjectHolder <MSetIterator *> ();
    _positioniterator = new XapianObjectHolder <PositionIterator *> ();
    _query = new XapianObjectHolder <Query *> ();
    _rset = new XapianObjectHolder <RSet *> ();
    _stem = new XapianObjectHolder <Stem *> ();
    _termiterator = new XapianObjectHolder <TermIterator *> ();

    return JNI_VERSION_1_2;
}

/**
 * Optional JNI function that java calls when it unloads this library.
 * However, I've never actually seen it get called on OS X, linux, or cygwin/win32
 */
void JNI_OnUnload(JavaVM *vm, void *reserved) {

}

