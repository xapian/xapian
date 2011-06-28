%{
/* java-swig/util.i: custom Java typemaps for xapian-bindings
 *
 * Copyright (c) 2007,2009 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

// In Java, we don't get SWIG_exception in the generated C++ wrapper sources.
# define XapianException(TYPE, MSG) SWIG_JavaException(jenv, TYPE, (MSG).c_str())
%}

// Use SWIG directors for Java wrappers.
#define XAPIAN_SWIG_DIRECTORS

// Rename function and method names to match Java conventions (e.g. from
// get_description() to getDescription()).
%rename("%(lctitle)s",%$isfunction) "";

// Fix up API methods which aren't split by '_' on word boundaries.

%rename("getTermPos") get_termpos;
%rename("getTermFreq") get_termfreq;
%rename("getTermWeight") get_termweight;
%rename("getDocCount") get_doccount;
%rename("getDocId") get_docid;
%rename("getDocLength") get_doclength;
%rename("getDocumentId") get_document_id;
%rename("positionListBegin") positionlist_begin;
%rename("positionListEnd") positionlist_end;
%rename("getValueNo") get_valueno;
%rename("termListCount") termlist_count;
%rename("termListBegin") termlist_begin;
%rename("termListEnd") termlist_end;
%rename("getFirstItem") get_firstitem;
%rename("getSumPart") get_sumpart;
%rename("getMaxPart") get_maxpart;
%rename("getSumExtra") get_sumextra;
%rename("getMaxExtra") get_maxextra;
%rename("postListBegin") postlist_begin;
%rename("postListEnd") postlist_end;
%rename("allTermsBegin") allterms_begin;
%rename("allTermsEnd") allterms_end;
%rename("getLastDocId") get_lastdocid;
%rename("getAvLength") get_avlength;
%rename("stopListBegin") stoplist_begin;
%rename("stopListEnd") stoplist_end;
%rename("getMSet") get_mset;
%rename("getESet") get_eset;

// Avoid collision with Object.clone
%rename("cloneWeight") clone;

// toString is more Java-esque and also matches what the old JNI bindings did.
%rename("toString") get_description() const;

// The old JNI bindings wrapped operator() as accept() for MatchDecider and
// ExpandDecider.
%rename("accept") Xapian::MatchDecider::operator();
%rename("accept") Xapian::ExpandDecider::operator();

// By default, valueno is wrapped as long and "(long, bool)" collides with
// some of SWIG/Java's machinery, so for now we wrap valueno as int to avoid
// this problem.
%apply int { Xapian::valueno };

%inline {
namespace Xapian {

// Wrap Xapian::version_string as Xapian.Version.String() as Java can't have
// functions outside a class and we don't want Xapian.Xapian.versionString()!
class Version {
  private:
    Version();
    ~Version();
  public:
    static const char * String() { return Xapian::version_string(); }
    static int Major() { return Xapian::major_version(); }
    static int Minor() { return Xapian::minor_version(); }
    static int Revision() { return Xapian::revision(); }
};

}
}

namespace Xapian {

%ignore version_string;
%ignore major_version;
%ignore minor_version;
%ignore revision;

// For compatibility with the original JNI wrappers.
// FIXME: These make use of the fact that the default ctor for PostingIterator,
// TermIterator, and ValueIterator produces an end iterator.

%extend PostingIterator {
    bool hasNext() const { return (*self) == Xapian::PostingIterator(); }
}

%extend TermIterator {
    bool hasNext() const { return (*self) == Xapian::TermIterator(); }
}

%extend ValueIterator {
    bool hasNext() const { return (*self) == Xapian::ValueIterator(); }
}

// FIXME: MSetIterator::hasNext() and ESetIterator::hasNext().

}

/* This tells SWIG to treat vector<string> as a special case when used as a
 * parameter in a function call. */
%typemap(in) const vector<string> & (vector<string> v, jsize len) {
    len = jenv->GetArrayLength($input);
    v.reserve(len);
    for (int i = 0; i < len; ++i) {
	jstring term = (jstring)jenv->GetObjectArrayElement($input, i);
	const char *c_term = jenv->GetStringUTFChars(term, 0);
	v.push_back(c_term);
	jenv->ReleaseStringUTFChars(term, c_term);
	jenv->DeleteLocalRef(term);
    }
    $1 = &v;
}

/* These 3 typemaps tell SWIG what JNI and Java types to use. */
%typemap(jni) const vector<string> & "jobjectArray"
%typemap(jtype) const vector<string> & "String[]"
%typemap(jstype) const vector<string> & "String[]"

/* This typemap handles the conversion of the jtype to jstype typemap type
 * and vice versa. */
%typemap(javain) const vector<string> & "$javainput"

%typemap(javacode) Xapian::Query %{
    // For compatibility with the original JNI wrappers.
    public final static op OP_AND = new op("OP_AND");
    public final static op OP_OR = new op("OP_OR");
    public final static op OP_AND_NOT = new op("OP_AND_NOT");
    public final static op OP_XOR = new op("OP_XOR");
    public final static op OP_AND_MAYBE = new op("OP_AND_MAYBE");
    public final static op OP_FILTER = new op("OP_FILTER");
    public final static op OP_NEAR = new op("OP_NEAR");
    public final static op OP_PHRASE = new op("OP_PHRASE");
    public final static op OP_ELITE_SET = new op("OP_ELITE_SET");
    public final static op OP_VALUE_RANGE = new op("OP_VALUE_RANGE");

    public final static Query MatchAll = new Query("");
    public final static Query MatchNothing = new Query();

    protected static long[] cArrayUnwrap(Query[] arrayWrapper) {
	long[] cArray = new long[arrayWrapper.length];
	for (int i=0; i<arrayWrapper.length; i++)
	    cArray[i] = Query.getCPtr(arrayWrapper[i]);
	return cArray;
    }
%}

/* This tells SWIG to treat vector<Xapian::Query> as a special case when used
 * as a parameter in a function call. */
%typemap(in) const vector<Xapian::Query> & (vector<Xapian::Query> v, jlong *jarr, jsize len) {
    if (!$input)
	SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "null array");
    len = jenv->GetArrayLength($input);
    v.reserve(len);
    jarr = jenv->GetLongArrayElements($input, NULL);
    if (!jarr) return $null;
    for (int i = 0; i < len; ++i) {
	Xapian::Query * query = *(Xapian::Query **)&jarr[i];
	v.push_back(*query);
    }
    /* We don't change the array so use JNI_ABORT to avoid any work
     * copying back non-existent changes if the JVM gave us a copy
     * of the array data. */
    jenv->ReleaseLongArrayElements($input, jarr, JNI_ABORT);
    $1 = &v;
}

/* These 3 typemaps tell SWIG what JNI and Java types to use. */
%typemap(jni) const vector<Xapian::Query> & "jlongArray"
%typemap(jtype) const vector<Xapian::Query> & "long[]"
%typemap(jstype) const vector<Xapian::Query> & "Query[]"

/* This typemap handles the conversion of the jstype to the jtype. */
%typemap(javain) const vector<Xapian::Query> & "Query.cArrayUnwrap($javainput)"

#if 0
#define XAPIAN_TERMITERATOR_PAIR_OUTPUT_TYPEMAP
%typemap(out) std::pair<Xapian::TermIterator, Xapian::TermIterator> {
    if (array_init($result) == FAILURE) {
	SWIG_PHP_Error(E_ERROR, "array_init failed");
    }

    for (Xapian::TermIterator i = $1.first; i != $1.second; ++i) {
	/* We have to cast away const here because the PHP API is rather
	 * poorly thought out - really there should be two API methods
	 * one of which takes a const char * and copies the string and
	 * the other which takes char * and takes ownership of the string.
	 *
	 * Passing 1 as the last parameter of add_next_index_stringl() tells
	 * PHP to copy the string pointed to by p, so it won't be modified.
	 */
	string term = *i;
	char *p = const_cast<char*>(term.data());
	add_next_index_stringl($result, p, term.length(), 1);
    }
}
#endif

#pragma SWIG nowarn=822 /* Suppress warning about covariant return types (FIXME - check if this is a problem!) */

/* vim:set syntax=cpp:set noexpandtab: */
