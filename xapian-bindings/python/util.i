%{
/* python/util.i: the Xapian scripting python interface helpers.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 James Aylett
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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
%}
%include typemaps.i
%include stl.i

%{
    namespace Xapian {
	class PythonProblem {};
	Query *get_py_query(PyObject *obj) {
	    Query *retval = 0;
	    PyObject *mythis = PyDict_GetItemString(((PyInstanceObject *)obj)
						    ->in_dict, "this");
	    if (SWIG_ConvertPtr(mythis,
				(void **)&retval,
				SWIGTYPE_p_Xapian__Query, 0)) {
		//	    cerr << "obj.this: " << PyString_AsString(mythis) << endl;
		//	    cerr << "Problem is: " << err << endl;
		PyErr_SetString(PyExc_ValueError,
				"Query object invalid");
		return 0;
	    }
	    return retval;
	}

	RSet *get_py_rset(PyObject *obj) {
	    RSet *retval = 0;
	    if (PyInstance_Check(obj)) {
		PyObject *mythis = PyDict_GetItemString(((PyInstanceObject *)obj)
							->in_dict, "this");
		if (SWIG_ConvertPtr(mythis,
				    (void **)&retval,
				    SWIGTYPE_p_Xapian__RSet, 0)) {
		    // cerr << "obj.this: " << PyString_AsString(mythis) << endl;
		    // cerr << "Problem is: " << err << endl;
		    PyErr_SetString(PyExc_ValueError,
				    "RSet object invalid");
		    return 0;
		}
	    }
	    return retval;
	}

#if 0 // FIXME
	MatchDecider *get_py_matchdecider(PyObject *obj) {
	    MatchDecider *retval = 0;
	    if (PyInstance_Check(obj)) {
		PyObject *mythis = PyDict_GetItemString(((PyInstanceObject *)obj)
							->in_dict, "this");
		if (SWIG_ConvertPtr(mythis, 
				    (void **)&retval,
				    SWIGTYPE_p_MatchDecider, 0)) {
		    // cerr << "obj.this: " << PyString_AsString(mythis) << endl;
		    // cerr << "Problem is: " << err << endl;
		    PyErr_SetString(PyExc_ValueError,
				    "MatchDecider object invalid");
		    return 0;
		}
	    }
	    return retval;
	}
#endif

	int get_py_int(PyObject *obj) {
	    if (!PyNumber_Check(obj)) {
		throw PythonProblem();
	    } else {
		return PyInt_AsLong(PyNumber_Int(obj));
	    }
	}
    };
%}

%typemap(python, in) const vector<Xapian::Query *> *(vector<Xapian::Query *> v){
    if (!PySequence_Check($input)) {
        PyErr_SetString(PyExc_TypeError, "expected list of queries");
        return NULL;
    }
    int i = 0;
    PyObject *obj;
    int sz = PySequence_Size($input);
    for (i=0; i<sz; i++) {
	obj = PySequence_GetItem($input, i);
	if (PyInstance_Check(obj)) {
	    Xapian::Query *subqp = Xapian::get_py_query(obj);
	    if (!subqp) {
		PyErr_SetString(PyExc_TypeError, "expected query");
		return NULL;
	    }
	    v.push_back(subqp);
	} else {
	    PyErr_SetString(PyExc_TypeError,
			    "expected instance objects");
	    return NULL;
	}
    }
    $1 = &v;
}
/*
%typemap(python, out) termname_list {
    $result = PyList_New(0);
    if ($result == 0) {
	return NULL;
    }

    Xapian::termname_list::const_iterator i = $1->begin();

    while (i!= $1->end()) {
        // FIXME: check return values (once we know what they should be)
        PyList_Append($result, PyString_FromString(i->c_str()));
	++i;
    }
    delete $1;
    $1 = 0;
}
*/
%typemap(python, in) const vector<string> &(vector<string> v){
    if (!PyList_Check($input)) {
        PyErr_SetString(PyExc_TypeError, "expected list");
        return NULL;
    }
    int numitems = PyList_Size($input);
    for (int i=0; i<numitems; ++i) {
        PyObject *obj = PyList_GetItem($input, i);
	if (PyString_Check(obj)) {
	    int len = PyString_Size(obj);
	    char *err = PyString_AsString(obj);
	    v.push_back(string(err, len));
	} else {
	    PyErr_SetString(PyExc_TypeError,
			    "expected list of strings");
	    return NULL;
	}
    }
    $1 = &v;
}

%typedef PyObject *LangSpecificListType;

#define OMMSET_DID 0
#define OMMSET_WT 1
#define OMMSET_RANK 2
#define OMMSET_PERCENT 3

#define OMESET_TNAME 0
#define OMESET_WT 1
%{
#define OMMSET_DID 0
#define OMMSET_WT 1
#define OMMSET_RANK 2
#define OMMSET_PERCENT 3

#define OMESET_TNAME 0
#define OMESET_WT 1

PyObject *Xapian_MSet_items_get(Xapian::MSet *mset)
{
    PyObject *retval = PyList_New(0);
    if (retval == 0) {
	return NULL;
    }

    Xapian::MSetIterator i = mset->begin();
    while (i != mset->end()) {
        PyObject *t = PyTuple_New(4);

	PyTuple_SetItem(t, OMMSET_DID, PyInt_FromLong(*i));
	PyTuple_SetItem(t, OMMSET_WT, PyFloat_FromDouble(i.get_weight()));
	PyTuple_SetItem(t, OMMSET_RANK, PyInt_FromLong(i.get_rank()));
	PyTuple_SetItem(t, OMMSET_PERCENT, PyInt_FromLong(i.get_percent()));

	PyList_Append(retval, t);
        ++i;
    }
    return retval;
}

PyObject *Xapian_ESet_items_get(Xapian::ESet *eset)
{
    PyObject *retval = PyList_New(0);
    if (retval == 0) {
	return NULL;
    }

    Xapian::ESetIterator i = eset->begin();
    while (i != eset->end()) {
        PyObject *t = PyTuple_New(2);

	PyTuple_SetItem(t, 0, PyString_FromString((*i).c_str()));
	PyTuple_SetItem(t, 1, PyFloat_FromDouble(i.get_weight()));

	PyList_Append(retval, t);
        ++i;
    }
    return retval;
}
%}

%typemap(python, memberout) PyObject *items {
    $result = PyList_New(0);
    if ($result == 0) {
	return NULL;
    }

    Xapian::MSetIterator i = $1.begin();
    while (i != $1.end()) {
        PyObject *t = PyTuple_New(2);

	PyTuple_SetItem(t, 0, PyInt_FromLong(*i));
	PyTuple_SetItem(t, 1, PyFloat_FromDouble(i->get_weight()));

	PyList_Append($result, t);
        ++i;
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
	    } else if (self->size() != other.size()) {
		return (self->size() < other.size())? -1 : 1;
	    }

	    for (size_t i=0; i<self->size(); ++i) {
		if ((*self)[i].get_weight() != other[i].get_weight()) {
		    return ((*self)[i].get_weight() < other[i].get_weight())? -1 : 1;
		} else if (*(*self)[i] != *other[i]) {
		    return (*(*self)[i] < *other[i])? -1 : 1;
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
};

namespace Xapian {
    %feature("director") MatchDecider;
    class MatchDecider {
    public:
	virtual int operator() (const Xapian::Document &doc) const = 0;
	virtual ~MatchDecider() { }
    };
};
