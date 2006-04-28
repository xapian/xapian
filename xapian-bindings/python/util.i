%{
/* python/util.i: custom Python typemaps for xapian-bindings
 *
 * Copyright (C) 1999,2000,2001 BrightStation PLC
 * Copyright (C) 2002 Ananova Ltd
 * Copyright (C) 2002,2003 James Aylett
 * Copyright (C) 2002,2003,2004,2005,2006 Olly Betts
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

%include typemaps.i
%include stl.i

%{
namespace Xapian {
    class PythonProblem {};
    Query *get_py_query(PyObject *obj) {
	PyObject * mythis = PyObject_GetAttrString(obj, "this");
	Query * retval = 0;
	if (!mythis || SWIG_ConvertPtr(mythis, (void **)&retval,
				       SWIGTYPE_p_Xapian__Query, 0) < 0) {
	    retval = 0;
	}
	return retval;
    }

#if 0 // Currently unused
    RSet *get_py_rset(PyObject *obj) {
	PyObject * mythis = PyObject_GetAttrString(obj, "this");
	Rset * retval = 0;
	if (!mythis || SWIG_ConvertPtr(mythis, (void **)&retval,
				       SWIGTYPE_p_Xapian__RSet, 0) < 0) {
	    retval = 0;
	}
	return retval;
    }
#endif

#if 0 // FIXME
    MatchDecider *get_py_matchdecider(PyObject *obj) {
	PyObject * mythis = PyObject_GetAttrString(obj, "this");
	MatchDecider * retval = 0;
	if (!mythis || SWIG_ConvertPtr(mythis, (void **)&retval,
				       SWIGTYPE_p_Xapian__MatchDecider, 0) < 0) {
	    retval = 0;
	}
	return retval;
    }
#endif

#if 0
    int get_py_int(PyObject *obj) {
	if (!PyNumber_Check(obj)) {
	    throw PythonProblem();
	}
	return PyInt_AsLong(PyNumber_Int(obj));
    }
#endif
}
%}

#define XAPIAN_MIXED_VECTOR_QUERY_INPUT_TYPEMAP
%typemap(typecheck, precedence=500) const vector<Xapian::Query> & {
    if (!PySequence_Check($input)) {
	$1 = 0;
    } else {
	$1 = 1;
	int numitems = PySequence_Size($input);
	for (int i = 0; i < numitems; ++i) {
	    PyObject *obj = PySequence_GetItem($input, i);
	    if (!PyString_Check(obj) && !Xapian::get_py_query(obj)) {
		$1 = 0;
		break;
	    }
	}
    }
}

%typemap(in) const vector<Xapian::Query> & (vector<Xapian::Query> v) {
    if (!PySequence_Check($input)) {
	PyErr_SetString(PyExc_TypeError, "expected list of strings or queries");
	return NULL;
    }

    int numitems = PySequence_Size($input);
    v.reserve(numitems);
    for (int i = 0; i < numitems; ++i) {
	PyObject *obj = PySequence_GetItem($input, i);
	if (PyString_Check(obj)) {
	    char * p;
	    int len;
	    /* We know this must be a string, so this call can't fail. */
	    (void)PyString_AsStringAndSize(obj, &p, &len);
	    v.push_back(Xapian::Query(string(p, len)));
	} else {
	    Xapian::Query *subqp = Xapian::get_py_query(obj);
	    if (!subqp) {
		PyErr_SetString(PyExc_TypeError, "expected string or query");
		return NULL;
	    }
	    v.push_back(*subqp);
	}
    }
    $1 = &v;
}

#define XAPIAN_TERMITERATOR_PAIR_OUTPUT_TYPEMAP
%typemap(out) std::pair<Xapian::TermIterator, Xapian::TermIterator> {
    $result = PyList_New(0);
    if ($result == 0) {
	return NULL;
    }

    for (Xapian::TermIterator i = $1.first; i != $1.second; ++i) {
	PyObject * str = PyString_FromStringAndSize((*i).data(), (*i).size());
	if (str == 0) return NULL;
	if (PyList_Append($result, str) == -1) return NULL;
    }
}

%typedef PyObject *LangSpecificListType;

#define MSET_DID 0
#define MSET_WT 1
#define MSET_RANK 2
#define MSET_PERCENT 3
#define MSET_DOCUMENT 4

#define ESET_TNAME 0
#define ESET_WT 1

%{
#define MSET_DID 0
#define MSET_WT 1
#define MSET_RANK 2
#define MSET_PERCENT 3
#define MSET_DOCUMENT 4

#define ESET_TNAME 0
#define ESET_WT 1

PyObject *Xapian_MSet_items_get(Xapian::MSet *mset)
{
    PyObject *retval = PyList_New(0);
    if (retval == 0) {
	return NULL;
    }

    for (Xapian::MSetIterator i = mset->begin(); i != mset->end(); ++i) {
	PyObject *t = PyTuple_New(4);
	if (!t) return NULL;

	PyTuple_SetItem(t, MSET_DID, PyInt_FromLong(*i));
	PyTuple_SetItem(t, MSET_WT, PyFloat_FromDouble(i.get_weight()));
	PyTuple_SetItem(t, MSET_RANK, PyInt_FromLong(i.get_rank()));
	PyTuple_SetItem(t, MSET_PERCENT, PyInt_FromLong(i.get_percent()));

	if (PyList_Append(retval, t) == -1) return NULL;
    }
    return retval;
}

PyObject *Xapian_ESet_items_get(Xapian::ESet *eset)
{
    PyObject *retval = PyList_New(0);
    if (retval == 0) {
	return NULL;
    }

    for (Xapian::ESetIterator i = eset->begin(); i != eset->end(); ++i) {
	PyObject *t = PyTuple_New(2);
	if (!t) return NULL;

	PyObject * str = PyString_FromStringAndSize((*i).data(), (*i).size());
	if (str == 0) return NULL;
	PyTuple_SetItem(t, ESET_TNAME, str);
	PyTuple_SetItem(t, ESET_WT, PyFloat_FromDouble(i.get_weight()));

	if (PyList_Append(retval, t) == -1) return NULL;
    }
    return retval;
}
%}

%typemap(memberout) PyObject *items {
    $result = PyList_New(0);
    if ($result == 0) {
	return NULL;
    }

    for (Xapian::MSetIterator i = $1.begin(); i != $1.end(); ++i) {
	PyObject *t = PyTuple_New(2);
	if (!t) return NULL;

	PyTuple_SetItem(t, MSET_DID, PyInt_FromLong(*i));
	PyTuple_SetItem(t, MSET_WT, PyFloat_FromDouble(i->get_weight()));

	if (PyList_Append($result, t) == -1) return NULL;
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

    %extend PositionIterator {
	bool __eq__(const PositionIterator &other) {
	    return (*self)==other;
	}
	bool __ne__(const PositionIterator &other) {
	    return (*self)!=other;
	}
    }

    %extend PostingIterator {
	bool __eq__(const PostingIterator &other) {
	    return (*self)==other;
	}
	bool __ne__(const PostingIterator &other) {
	    return (*self)!=other;
	}
    }

    %extend ValueIterator {
	bool __eq__(const ValueIterator &other) {
	    return (*self)==other;
	}
	bool __ne__(const ValueIterator &other) {
	    return (*self)!=other;
	}
    }

    %extend MSetIterator {
	bool __eq__(const MSetIterator &other) {
	    return (*self)==other;
	}
	bool __ne__(const MSetIterator &other) {
	    return (*self)!=other;
	}
    }

    %extend ESetIterator {
	bool __eq__(const ESetIterator &other) {
	    return (*self)==other;
	}
	bool __ne__(const ESetIterator &other) {
	    return (*self)!=other;
	}
    }

    %extend MSet {
	%immutable;
	// access to the items array
	PyObject *items;

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
	%mutable;
    }

    //%apply LangSpecificListType items { PyObject *items }

    %extend ESet {
	%immutable;
	PyObject *items;
	%mutable;
    }
}

#pragma SWIG nowarn=515 /* Suppress warning that const is discarded by operator() */
namespace Xapian {
    %feature("director") MatchDecider;
    class MatchDecider {
    public:
	virtual int operator() (const Xapian::Document &doc) const = 0;
	virtual ~MatchDecider() { }
    };
}
#pragma SWIG nowarn=
