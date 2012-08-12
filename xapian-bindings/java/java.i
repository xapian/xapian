%module(directors="1") xapian
%{
/* java.i: SWIG interface file for the Java bindings
 *
 * Copyright (c) 2007,2009,2011,2012 Olly Betts
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

%include ../xapian-head.i

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

#define XAPIAN_MIXED_SUBQUERIES_BY_ITERATOR_TYPEMAP

%{
class XapianSWIGStrItor {
    JNIEnv * jenv;

    jobjectArray array;

    jsize i;

  public:
    typedef std::random_access_iterator_tag iterator_category;
    typedef Xapian::Query value_type;
    typedef Xapian::termcount_diff difference_type;
    typedef Xapian::Query * pointer;
    typedef Xapian::Query & reference;

    XapianSWIGStrItor() { }

    void begin(JNIEnv * jenv_, jobjectArray array_) {
        jenv = jenv_;
        array = array_;
        i = 0;
    }

    void end(jsize len_) {
        i = len_;
    }

    XapianSWIGStrItor & operator++() {
	++i;
	return *this;
    }

    Xapian::Query operator*() const {
	jstring term = (jstring)jenv->GetObjectArrayElement(array, i);
	const char *c_term = jenv->GetStringUTFChars(term, 0);
	Xapian::Query subq(c_term);
	jenv->ReleaseStringUTFChars(term, c_term);
	jenv->DeleteLocalRef(term);
	return subq;
    }

    bool operator==(const XapianSWIGStrItor & o) {
	return i == o.i;
    }

    bool operator!=(const XapianSWIGStrItor & o) {
	return !(*this == o);
    }

    difference_type operator-(const XapianSWIGStrItor &o) const {
        return i - o.i;
    }
};

class XapianSWIGQueryItor {
    jlong * p;

  public:
    typedef std::random_access_iterator_tag iterator_category;
    typedef Xapian::Query value_type;
    typedef Xapian::termcount_diff difference_type;
    typedef Xapian::Query * pointer;
    typedef Xapian::Query & reference;

    XapianSWIGQueryItor() { }

    void set_p(jlong * p_) { p = p_; }

    XapianSWIGQueryItor & operator++() {
	++p;
	return *this;
    }

    const Xapian::Query & operator*() const {
	return **(Xapian::Query **)p;
    }

    bool operator==(const XapianSWIGQueryItor & o) {
	return p == o.p;
    }

    bool operator!=(const XapianSWIGQueryItor & o) {
	return !(*this == o);
    }

    difference_type operator-(const XapianSWIGQueryItor &o) const {
        return p - o.p;
    }
};

%}

%typemap(in) (XapianSWIGStrItor qbegin, XapianSWIGStrItor qend) {
    $1.begin(jenv, $input);
    $2.end(jenv->GetArrayLength($input));
}

/* These 3 typemaps tell SWIG what JNI and Java types to use. */
%typemap(jni) XapianSWIGStrItor, XapianSWIGStrItor "jobjectArray"
%typemap(jtype) XapianSWIGStrItor, XapianSWIGStrItor "String[]"
%typemap(jstype) XapianSWIGStrItor, XapianSWIGStrItor "String[]"

/* This typemap handles the conversion of the jtype to jstype typemap type
 * and vice versa. */
%typemap(javain) XapianSWIGStrItor, XapianSWIGStrItor "$javainput"

%typemap(in) (XapianSWIGQueryItor qbegin, XapianSWIGQueryItor qend) %{
    if (!$input)
	SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "null array");
    jlong * jarr = jenv->GetLongArrayElements($input, NULL);
    if (!jarr) return $null;
    $1.set_p(jarr);
    $2.set_p(jarr + jenv->GetArrayLength($input));
%}

%typemap(freearg) (XapianSWIGQueryItor qbegin, XapianSWIGQueryItor qend) %{
    /* We don't change the array so use JNI_ABORT to avoid any work
     * copying back non-existent changes if the JVM gave us a copy
     * of the array data. */
    jenv->ReleaseLongArrayElements($input, jarr, JNI_ABORT);
%}

/* These 3 typemaps tell SWIG what JNI and Java types to use. */
%typemap(jni) XapianSWIGQueryItor, XapianSWIGQueryItor "jlongArray"
%typemap(jtype) XapianSWIGQueryItor, XapianSWIGQueryItor "long[]"
%typemap(jstype) XapianSWIGQueryItor, XapianSWIGQueryItor "Query[]"

/* This typemap handles the conversion of the jstype to the jtype. */
%typemap(javain) XapianSWIGQueryItor, XapianSWIGQueryItor "Query.cArrayUnwrap($javainput)"

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

#define XAPIAN_TERMITERATOR_PAIR_OUTPUT_TYPEMAP
%typemap(out) std::pair<Xapian::TermIterator, Xapian::TermIterator> {
    jobjectArray c_result;
    jboolean jbool;
    jstring temp_string;
    int n = 0;
    const jclass clazz = jenv->FindClass("java/lang/String");
    const jclass arrayClass = jenv->FindClass("java/util/ArrayList");
    if (arrayClass == NULL)
	return NULL;

    const jmethodID mid_init = jenv->GetMethodID(arrayClass, "<init>", "()V");
    if (mid_init == NULL)
	return NULL;

    jobject objArr = jenv->NewObject(arrayClass, mid_init);
    if (objArr == NULL)
	return NULL;

    const jmethodID mid_add = jenv->GetMethodID(arrayClass, "add",
						"(Ljava/lang/Object;)Z");
    if (mid_add == NULL)
	return NULL;

    const jmethodID mid_toArray = jenv->GetMethodID(arrayClass, "toArray", "([Ljava/lang/Object;)[Ljava/lang/Object;");
    if (mid_toArray == NULL) return NULL;

    for (Xapian::TermIterator i = $1.first; i != $1.second; ++i) {
	temp_string = jenv->NewStringUTF((*i).c_str());
	jbool = jenv->CallBooleanMethod(objArr, mid_add, temp_string);
	if (jbool == false) return NULL;
	jenv->DeleteLocalRef(temp_string);
	++n;
    }

    c_result = jenv->NewObjectArray(n, clazz, NULL);
    $result = (jobjectArray)jenv->CallObjectMethod(objArr, mid_toArray, c_result);
}

%typemap(jni) std::pair<Xapian::TermIterator, Xapian::TermIterator> "jobjectArray"
%typemap(jtype) std::pair<Xapian::TermIterator, Xapian::TermIterator> "String[]"
%typemap(jstype) std::pair<Xapian::TermIterator, Xapian::TermIterator> "String[]"

/* This typemap handles the conversion of the jstype to the jtype. */
%typemap(javaout) std::pair<Xapian::TermIterator, Xapian::TermIterator> { return $jnicall; }


#pragma SWIG nowarn=822 /* Suppress warning about covariant return types (FIXME - check if this is a problem!) */

%include ../generic/except.i
%include ../xapian-headers.i
%include ../fake_dbfactory.i
