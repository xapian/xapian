%{
/* python/util.i: custom Python typemaps for xapian-bindings
 *
 * Copyright (C) 1999,2000,2001 BrightStation PLC
 * Copyright (C) 2002 Ananova Ltd
 * Copyright (C) 2002,2003 James Aylett
 * Copyright (C) 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011 Olly Betts
 * Copyright (C) 2007 Lemur Consulting Ltd
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
%}

/* Include the documentation comments extracted from doxygen output. */
#ifdef DOCCOMMENTS_I_SOURCES
%include "doccomments.i"
#endif

/* Include overrides for the documentation comments. */
%include "extracomments.i"

%include typemaps.i
%include stl.i

/* Wrap get_description() methods as str(). */
%rename(__str__) get_description;

/* Hide "unsafe" C++ iterator methods. */
%rename(_allterms_begin) Xapian::Database::allterms_begin;
%rename(_allterms_end) Xapian::Database::allterms_end;
%rename(_metadata_keys_begin) Xapian::Database::metadata_keys_begin;
%rename(_metadata_keys_end) Xapian::Database::metadata_keys_end;
%rename(_synonym_keys_begin) Xapian::Database::synonym_keys_begin;
%rename(_synonym_keys_end) Xapian::Database::synonym_keys_end;
%rename(_synonyms_begin) Xapian::Database::synonyms_begin;
%rename(_synonyms_end) Xapian::Database::synonyms_end;
%rename(_spellings_begin) Xapian::Database::spellings_begin;
%rename(_spellings_end) Xapian::Database::spellings_end;
%rename(_positionlist_begin) Xapian::Database::positionlist_begin;
%rename(_positionlist_end) Xapian::Database::positionlist_end;
%rename(_postlist_begin) Xapian::Database::postlist_begin;
%rename(_postlist_end) Xapian::Database::postlist_end;
%rename(_termlist_begin) Xapian::Database::termlist_begin;
%rename(_termlist_end) Xapian::Database::termlist_end;
%rename(_termlist_begin) Xapian::Document::termlist_begin;
%rename(_termlist_end) Xapian::Document::termlist_end;
%rename(_values_begin) Xapian::Document::values_begin;
%rename(_values_end) Xapian::Document::values_end;
%rename(_get_matching_terms_begin) Xapian::Enquire::get_matching_terms_begin;
%rename(_get_matching_terms_end) Xapian::Enquire::get_matching_terms_end;
%rename(_begin) Xapian::ESet::begin;
%rename(_end) Xapian::ESet::end;
%rename(_begin) Xapian::MSet::begin;
%rename(_end) Xapian::MSet::end;
%rename(_positionlist_begin) Xapian::PostingIterator::positionlist_begin;
%rename(_positionlist_end) Xapian::PostingIterator::positionlist_end;
%rename(_get_terms_begin) Xapian::Query::get_terms_begin;
%rename(_get_terms_end) Xapian::Query::get_terms_end;
%rename(_stoplist_begin) Xapian::QueryParser::stoplist_begin;
%rename(_stoplist_end) Xapian::QueryParser::stoplist_end;
%rename(_unstem_begin) Xapian::QueryParser::unstem_begin;
%rename(_unstem_end) Xapian::QueryParser::unstem_end;
%rename(_positionlist_begin) Xapian::TermIterator::positionlist_begin;
%rename(_positionlist_end) Xapian::TermIterator::positionlist_end;

/* We replace the get_hit() method with one which returns an MSetitem. */
%rename(_get_hit_internal) Xapian::MSet::get_hit;

/* Force xapian.BAD_VALUENO to be handled as a constant rather than a
 * read-only variable (ticket#297).
 */
%ignore Xapian::BAD_VALUENO;
%constant Xapian::valueno BAD_VALUENO = Xapian::BAD_VALUENO;

%{
namespace Xapian {
    Query *get_py_query(PyObject *obj) {
#if PY_VERSION_HEX < 0x02050000
	// In Python 2.4 (and presumably earlier), PyObject_GetAttrString()
	// takes a char* parameter which causes a warning with GCC >= 4.2.
	// This is fixed in Python 2.5.
	PyObject * mythis = PyObject_GetAttrString(obj, (char *)"this");
#else
	PyObject * mythis = PyObject_GetAttrString(obj, "this");
#endif
	if (!mythis)
	    return 0;

	Query * retval = 0;
	int res = SWIG_ConvertPtr(mythis, (void **)&retval,
				  SWIGTYPE_p_Xapian__Query, 0);
	if (!SWIG_IsOK(res)) {
	    retval = 0;
	}
	Py_DECREF(mythis);
	return retval;
    }
}
%}

#define XAPIAN_MIXED_VECTOR_QUERY_INPUT_TYPEMAP
%typemap(typecheck, precedence=500) const vector<Xapian::Query> & {
    if (!PySequence_Check($input)) {
	$1 = 0;
    } else {
	$1 = 1;
	PyObject * fastseq = PySequence_Fast($input, "expected sequence of strings or queries");
	if (!fastseq) {
	    // We've already checked that we have a sequence, so the failure of
	    // PySequence_Fast() is a serious error, not just a failure of
	    // typecheck.
	    SWIG_fail;
	}
	int numitems = PySequence_Fast_GET_SIZE(fastseq);
	for (int i = 0; i < numitems; ++i) {
	    PyObject *obj = PySequence_Fast_GET_ITEM(fastseq, i);
	    if (!PyUnicode_Check(obj) &&
%#if PY_VERSION_HEX >= 0x03000000
		!PyBytes_Check(obj) &&
%#else
		!PyString_Check(obj) &&
%#endif
		!Xapian::get_py_query(obj)) {
		$1 = 0;
		break;
	    }
	}
	Py_DECREF(fastseq);
    }
}

%typemap(in) const vector<Xapian::Query> & (vector<Xapian::Query> v) {
    PyObject * fastseq = PySequence_Fast($input, "expected sequence of strings or queries");
    if (!fastseq) {
	SWIG_fail;
    }

    int numitems = PySequence_Fast_GET_SIZE(fastseq);
    v.reserve(numitems);
    for (int i = 0; i < numitems; ++i) {
	PyObject *obj = PySequence_Fast_GET_ITEM(fastseq, i);
	PyObject *decrefme = NULL;
	if (PyUnicode_Check(obj)) {
	    PyObject *strobj = PyUnicode_EncodeUTF8(PyUnicode_AS_UNICODE(obj), PyUnicode_GET_SIZE(obj), "ignore");
	    if (!strobj) SWIG_fail;
	    obj = strobj;
	    decrefme = strobj;
	}
%#if PY_VERSION_HEX >= 0x03000000
	if (PyBytes_Check(obj))
%#else
	if (PyString_Check(obj))
%#endif
	{
	    char * p;
	    Py_ssize_t len;
%#if PY_VERSION_HEX >= 0x03000000
	    /* We know this must be a bytes object, so this call can't fail. */
	    (void)PyBytes_AsStringAndSize(obj, &p, &len);
%#else
	    /* We know this must be a string, so this call can't fail. */
	    (void)PyString_AsStringAndSize(obj, &p, &len);
%#endif
	    v.push_back(Xapian::Query(string(p, len)));
	} else {
	    Xapian::Query *subqp = Xapian::get_py_query(obj);
	    if (!subqp) {
		PyErr_SetString(PyExc_TypeError, "expected string or query");
		Py_XDECREF(decrefme);
		SWIG_fail;
	    }
	    v.push_back(*subqp);
	}
	Py_XDECREF(decrefme);
    }
    $1 = &v;
    Py_DECREF(fastseq);
}

%typedef PyObject *LangSpecificListType;

%inline %{
#define MSET_DID 0
#define MSET_WT 1
#define MSET_RANK 2
#define MSET_PERCENT 3
#define MSET_DOCUMENT 4

#define ESET_TNAME 0
#define ESET_WT 1
%}

%feature("nothread") Xapian::MSet::items;
%{
/* The GIL must be held when this is called. */
PyObject *Xapian_MSet_items_get(Xapian::MSet *mset)
{
    PyObject *retval = PyList_New(mset->size());
    if (retval == 0) {
	return NULL;
    }

    Py_ssize_t idx = 0;
    for (Xapian::MSetIterator i = mset->begin(); i != mset->end(); ++i) {
	PyObject *t = PyTuple_New(4);
	if (!t) {
	    Py_DECREF(retval);
	    return NULL;
	}

	PyList_SET_ITEM(retval, idx++, t);

	PyTuple_SET_ITEM(t, MSET_DID, PyInt_FromLong(*i));
	PyTuple_SET_ITEM(t, MSET_WT, PyFloat_FromDouble(i.get_weight()));
	PyTuple_SET_ITEM(t, MSET_RANK, PyInt_FromLong(i.get_rank()));
	PyTuple_SET_ITEM(t, MSET_PERCENT, PyInt_FromLong(i.get_percent()));
    }
    return retval;
}
%}

%feature("nothread") Xapian::ESet::items;
%{
/* The GIL must be held when this is called. */
PyObject *Xapian_ESet_items_get(Xapian::ESet *eset)
{
    PyObject *retval = PyList_New(eset->size());
    if (retval == 0) {
	return NULL;
    }

    Py_ssize_t idx = 0;
    for (Xapian::ESetIterator i = eset->begin(); i != eset->end(); ++i) {
	PyObject *t = PyTuple_New(2);
	if (!t) {
	    Py_DECREF(retval);
	    return NULL;
	}

	PyList_SET_ITEM(retval, idx++, t);

#if PY_VERSION_HEX >= 0x03000000
	PyObject * str = PyBytes_FromStringAndSize((*i).data(), (*i).size());
#else
	PyObject * str = PyString_FromStringAndSize((*i).data(), (*i).size());
#endif
	if (str == 0) {
	    Py_DECREF(retval);
	    return NULL;
	}

	PyTuple_SET_ITEM(t, ESET_TNAME, str);
	PyTuple_SET_ITEM(t, ESET_WT, PyFloat_FromDouble(i.get_weight()));
    }
    return retval;
}
%}

namespace Xapian {
    %extend TermIterator {
	bool __eq__(const TermIterator &other) {
	    return (*self)==other;
	}
	bool __ne__(const TermIterator &other) {
	    return (*self)!=other;
	}
    }
    %rename(_TermIterator) TermIterator;

    %extend PositionIterator {
	bool __eq__(const PositionIterator &other) {
	    return (*self)==other;
	}
	bool __ne__(const PositionIterator &other) {
	    return (*self)!=other;
	}
    }
    %rename(_PositionIterator) PositionIterator;

    %extend PostingIterator {
	bool __eq__(const PostingIterator &other) {
	    return (*self)==other;
	}
	bool __ne__(const PostingIterator &other) {
	    return (*self)!=other;
	}
    }
    %rename(_PostingIterator) PostingIterator;

    %extend ValueIterator {
	bool __eq__(const ValueIterator &other) {
	    return (*self)==other;
	}
	bool __ne__(const ValueIterator &other) {
	    return (*self)!=other;
	}
    }
    %rename(_ValueIterator) ValueIterator;

    %extend MSetIterator {
	bool __eq__(const MSetIterator &other) {
	    return (*self)==other;
	}
	bool __ne__(const MSetIterator &other) {
	    return (*self)!=other;
	}
    }
    %rename(_MSetIterator) MSetIterator;

    %extend ESetIterator {
	bool __eq__(const ESetIterator &other) {
	    return (*self)==other;
	}
	bool __ne__(const ESetIterator &other) {
	    return (*self)!=other;
	}
    }
    %rename(_ESetIterator) ESetIterator;

    %extend MSet {
	%immutable;
	// access to the items array
	PyObject *items;
	%mutable;

	// for comparison
	int __cmp__(const MSet &other) {
	    if (self->get_max_possible() != other.get_max_possible()) {
		return (self->get_max_possible() < other.get_max_possible())? -1 : 1;
	    }
	    if (self->size() != other.size()) {
		return (self->size() < other.size())? -1 : 1;
	    }

	    for (size_t i=0; i<self->size(); ++i) {
		if (*(*self)[i] != *other[i]) {
		    return (*(*self)[i] < *other[i])? -1 : 1;
		}
		if ((*self)[i].get_weight() != other[i].get_weight()) {
		    return ((*self)[i].get_weight() < other[i].get_weight())? -1 : 1;
		}
	    }
	    return 0;
	}
    }

    //%apply LangSpecificListType items { PyObject *items }

    %extend ESet {
	%immutable;
	PyObject *items;
	%mutable;
    }
}

%{
/* Forward declaration. */
SWIGINTERN int
SWIG_AsPtr_std_string (PyObject * obj, std::string **val);

/* Utility function which works like SWIG_AsPtr_std_string, but
 * converts unicode strings to UTF-8 simple strings first. */
SWIGINTERN int
SWIG_anystring_as_ptr(PyObject ** obj, std::string **val)
{
    if (PyUnicode_Check(*obj)) {
	PyObject * strobj = PyUnicode_EncodeUTF8(PyUnicode_AS_UNICODE(*obj), PyUnicode_GET_SIZE(*obj), "ignore");
	if (strobj == NULL) return SWIG_ERROR;
	int res = SWIG_AsPtr_std_string(strobj, val);
	Py_DECREF(strobj);
	return res;
    } else {
	return SWIG_AsPtr_std_string(*obj, val);
    }
}
%}

/* These typemaps depends somewhat heavily on the internals of SWIG, so
 * might break with future versions of SWIG.
 */
%typemap(in) const std::string &(int res = SWIG_OLDOBJ) {
    std::string *ptr = (std::string *)0;
    res = SWIG_anystring_as_ptr(&($input), &ptr);
    if (!SWIG_IsOK(res)) {
	%argument_fail(res,"$type",$symname, $argnum); 
    }
    if (!ptr) {
	%argument_nullref("$type",$symname, $argnum); 
    }
    $1 = ptr;
}
%typemap(in) std::string {
    std::string *ptr = (std::string *)0;
    int res = SWIG_anystring_as_ptr(&($input), &ptr);
    if (!SWIG_IsOK(res) || !ptr) {
	%argument_fail((ptr ? res : SWIG_TypeError),"$type",$symname, $argnum); 
    }
    $1 = *ptr;
    if (SWIG_IsNewObj(res)) delete ptr;
}
%typemap(freearg,noblock=1,match="in") const std::string & {
    if (SWIG_IsNewObj(res$argnum)) %delete($1);
}
%typemap(typecheck,noblock=1,precedence=900) const std::string & {
    int res = SWIG_anystring_as_ptr(&($input), (std::string**)(0));
    $1 = SWIG_CheckState(res);

}

/* This typemap is only currently needed for returning a value from the
 * get_description() method of a Stopper subclass to a C++ caller, but might be
 * more generally useful in future.
 */
%typemap(directorout,noblock=1) std::string {
    std::string *swig_optr = 0;
    int swig_ores;
    {
	PyObject * tmp = $input;
	Py_INCREF(tmp);
	swig_ores = SWIG_anystring_as_ptr(&tmp, &swig_optr);
	Py_DECREF(tmp);
    }
    if (!SWIG_IsOK(swig_ores) || !swig_optr) {
	%dirout_fail((swig_optr ? swig_ores : SWIG_TypeError),"$type");
    }
    $result = *swig_optr;
    if (SWIG_IsNewObj(swig_ores)) %delete(swig_optr);
}

/** This pair of typemaps implements conversion of the return value of
 *  ValueRangeProcessor subclasses implemented in Python from a tuple of
 *  (valueno, begin, end) to a return value of valueno, and assigning the new
 *  values of begin and end to the parameters.
 */
%typemap(directorin,noblock=1) std::string & {
    $input = SWIG_From_std_string(static_cast< std::string >($1_name));
}
%typemap(directorout,noblock=1) Xapian::valueno {
    if (!PyTuple_Check($input)) {
        %dirout_fail(SWIG_TypeError, "($type, std::string, std::string)");
    }
    if (PyTuple_Size($input) != 3) {
        %dirout_fail(SWIG_IndexError, "($type, std::string, std::string)");
    }

    // Set the return value from the first item of the tuple.
    unsigned int swig_val;
    int swig_res = SWIG_AsVal_unsigned_SS_int(PyTuple_GET_ITEM((PyObject *)$input, 0), &swig_val);
    if (!SWIG_IsOK(swig_res)) {
        %dirout_fail(swig_res, "($type, std::string, std::string)");
    }
    c_result = static_cast< Xapian::valueno >(swig_val);

    // Set "begin" from the second item of the tuple.
    std::string *ptr = (std::string *)0;
    swig_res = SWIG_AsPtr_std_string(PyTuple_GET_ITEM((PyObject *)$input, 1), &ptr);
    if (!SWIG_IsOK(swig_res) || !ptr) {
        delete ptr;
        ptr = (std::string *)0;
	%dirout_fail((ptr ? swig_res : SWIG_TypeError),"($type, std::string, std::string)"); 
    }
    begin = *ptr;
    delete ptr;
    ptr = (std::string *)0;

    // Set "end" from the third item of the tuple.
    swig_res = SWIG_AsPtr_std_string(PyTuple_GET_ITEM((PyObject *)$input, 2), &ptr);
    if (!SWIG_IsOK(swig_res) || !ptr) {
        delete ptr;
        ptr = (std::string *)0;
	%dirout_fail((ptr ? swig_res : SWIG_TypeError),"($type, std::string, std::string)"); 
    }
    end = *ptr;
    delete ptr;
    ptr = (std::string *)0;
}

/* These typemaps handle ValueRangeProcessors, which take non-const references
 * to std::string and modify the strings.
 */
%typemap(in) std::string &begin (std::string temp),
             std::string &end (std::string temp) {
    std::string *ptr = (std::string *)0;
    int res = SWIG_AsPtr_std_string($input, &ptr);
    if (!SWIG_IsOK(res) || !ptr) {
	%argument_fail((ptr ? res : SWIG_TypeError),"$type",$symname, $argnum); 
    }
    temp = *ptr;
    $1 = &temp;
    if (SWIG_IsNewObj(res)) delete ptr;
}
%typemap(argout) (std::string &begin, std::string &end) {
    PyObject * str;
    PyObject * newresult;

    // Put the existing result into the first item of a new 3-tuple.
    newresult = PyTuple_New(3);
    if (newresult == 0) {
        Py_DECREF($result);
        $result = NULL;
        SWIG_fail;
    }
    PyTuple_SET_ITEM(newresult, 0, $result);
    $result = newresult;

%#if PY_VERSION_HEX >= 0x03000000
    str = PyBytes_FromStringAndSize($1->data(), $1->size());
%#else
    str = PyString_FromStringAndSize($1->data(), $1->size());
%#endif
    if (str == 0) {
        Py_DECREF($result);
        $result = NULL;
        SWIG_fail;
    }
    PyTuple_SET_ITEM($result, 1, str);

%#if PY_VERSION_HEX >= 0x03000000
    str = PyBytes_FromStringAndSize($2->data(), $2->size());
%#else
    str = PyString_FromStringAndSize($2->data(), $2->size());
%#endif
    if (str == 0) {
        Py_DECREF($result);
        $result = NULL;
        SWIG_fail;
    }

    PyTuple_SET_ITEM($result, 2, str);
}

%typemap(directorin) (size_t num_tags, const std::string tags[]) {
    PyObject * result = PyList_New(num_tags);
    if (result == 0) {
	return NULL;
    }

    for (size_t i = 0; i != num_tags; ++i) {
%#if PY_VERSION_HEX >= 0x03000000
	PyObject * str = PyBytes_FromStringAndSize(tags[i].data(), tags[i].size());
%#else
	PyObject * str = PyString_FromStringAndSize(tags[i].data(), tags[i].size());
%#endif
	if (str == 0) {
	    Py_DECREF(result);
	    return NULL;
	}

	PyList_SET_ITEM(result, i, str);
    }
    $input = result;
}

/* vim:set syntax=cpp:set noexpandtab: */
