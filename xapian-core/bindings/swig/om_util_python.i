%{
/* om_util_python.i: the Open Muscat scripting python interface helpers.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

%typemap(python, out) string {
    $target = PyString_FromString(($source)->c_str());
    delete $source;
    $source = 0;
}

%typemap(python, in) const string &(string temp) {
    if (PyString_Check($source)) {
	temp = string(PyString_AsString($source),
		      PyString_Size($source));
	$target = &temp;
    } else {
        PyErr_SetString(PyExc_TypeError, "string expected");
	return NULL;
    }
}

%{
    class OmPythonProblem {};
    OmQuery *get_py_omquery(PyObject *obj)
    {
	OmQuery *retval = 0;
	PyObject *mythis = PyDict_GetItemString(((PyInstanceObject *)obj)
						->in_dict, "this");
	if (char *err = SWIG_GetPtr(PyString_AsString(mythis),
				    (void **)&retval,
				    "_OmQuery_p")) {
	    cerr << "obj.this: " << PyString_AsString(mythis) << endl;
	    cerr << "Problem is: " << err << endl;
	    PyErr_SetString(PyExc_ValueError,
			    "OmQuery object invalid");
	    return 0;
	}
	return retval;
    }

    OmRSet *get_py_omrset(PyObject *obj)
    {
	OmRSet *retval = 0;
	if (PyInstance_Check(obj)) {
	    PyObject *mythis = PyDict_GetItemString(((PyInstanceObject *)obj)
						    ->in_dict, "this");
	    if (char *err = SWIG_GetPtr(PyString_AsString(mythis),
					(void **)&retval,
					"_OmRSet_p")) {
		cerr << "obj.this: " << PyString_AsString(mythis) << endl;
		cerr << "Problem is: " << err << endl;
		PyErr_SetString(PyExc_ValueError,
				"OmRSet object invalid");
		return 0;
	    }
	}
	return retval;
    }

    OmMatchOptions *get_py_ommatchoptions(PyObject *obj)
    {
	OmMatchOptions *retval = 0;
	if (PyInstance_Check(obj)) {
	    PyObject *mythis = PyDict_GetItemString(((PyInstanceObject *)obj)
						    ->in_dict, "this");
	    if (char *err = SWIG_GetPtr(PyString_AsString(mythis),
					(void **)&retval,
					"_OmMatchOptions_p")) {
		cerr << "obj.this: " << PyString_AsString(mythis) << endl;
		cerr << "Problem is: " << err << endl;
		PyErr_SetString(PyExc_ValueError,
				"OmMatchOptions object invalid");
		return 0;
	    }
	}
	return retval;
    }

    OmMatchDecider *get_py_ommatchdecider(PyObject *obj)
    {
	OmMatchDecider *retval = 0;
	if (PyInstance_Check(obj)) {
	    PyObject *mythis = PyDict_GetItemString(((PyInstanceObject *)obj)
						    ->in_dict, "this");
	    if (char *err = SWIG_GetPtr(PyString_AsString(mythis),
					(void **)&retval,
					"_OmMatchDecider_p")) {
		cerr << "obj.this: " << PyString_AsString(mythis) << endl;
		cerr << "Problem is: " << err << endl;
		PyErr_SetString(PyExc_ValueError,
				"OmMatchDecider object invalid");
		return 0;
	    }
	}
	return retval;
    }

    int get_py_int(PyObject *obj) {
	if (!PyNumber_Check(obj)) {
	    throw OmPythonProblem();
	} else {
	    return PyInt_AsLong(PyNumber_Int(obj));
	}
    }
%}

%typemap(python, in) const vector<OmQuery *> *(vector<OmQuery *> v){
    if (!PyList_Check($source)) {
        PyErr_SetString(PyExc_TypeError, "expected list");
        return NULL;
    }
    int numitems = PyList_Size($source);
    for (int i=0; i<numitems; ++i) {
        PyObject *obj = PyList_GetItem($source, i);
	if (PyInstance_Check(obj)) {
	    OmQuery *subqp = get_py_omquery(obj);
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
    $target = &v;
}

%typemap(python, out) om_termname_list {
    $target = PyList_New(0);
    if ($target == 0) {
	return NULL;
    }

    om_termname_list::const_iterator i = $source->begin();

    while (i!= $source->end()) {
        // FIXME: check return values (once we know what they should be)
        PyList_Append($target, PyString_FromString(i->c_str()));
	++i;
    }
    delete $source;
    $source = 0;
}

%typemap(python, in) const vector<string> &(vector<string> v){
    if (!PyList_Check($source)) {
        PyErr_SetString(PyExc_TypeError, "expected list");
        return NULL;
    }
    int numitems = PyList_Size($source);
    for (int i=0; i<numitems; ++i) {
        PyObject *obj = PyList_GetItem($source, i);
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
    $target = &v;
}

%typedef PyObject *LangSpecificListType;

#define OMMSET_DID 0
#define OMMSET_WT 1
#define OMMSET_COLLAPSEKEY 2
%{
#define OMMSET_DID 0
#define OMMSET_WT 1
#define OMMSET_COLLAPSEKEY 2

PyObject *OmMSet_items_get(OmMSet *mset)
{
    PyObject *retval = PyList_New(0);
    if (retval == 0) {
	return NULL;
    }

    vector<OmMSetItem>::const_iterator i = mset->items.begin();
    while (i != mset->items.end()) {
        PyObject *t = PyTuple_New(3);

	PyTuple_SetItem(t, OMMSET_DID, PyInt_FromLong(i->did));
	PyTuple_SetItem(t, OMMSET_WT, PyFloat_FromDouble(i->wt));
	PyTuple_SetItem(t, OMMSET_COLLAPSEKEY, PyString_FromString(i->collapse_key.value.c_str()));

	PyList_Append(retval, t);
        ++i;
    }
    return retval;
}

PyObject *OmESet_items_get(OmESet *eset)
{
    PyObject *retval = PyList_New(0);
    if (retval == 0) {
	return NULL;
    }

    vector<OmESetItem>::const_iterator i = eset->items.begin();
    while (i != eset->items.end()) {
        PyObject *t = PyTuple_New(2);

	PyTuple_SetItem(t, 0, PyString_FromString((i->tname).c_str()));
	PyTuple_SetItem(t, 1, PyFloat_FromDouble(i->wt));

	PyList_Append(retval, t);
        ++i;
    }
    return retval;
}
%}

%typemap(python, memberout) PyObject *items {
    $target = PyList_New(0);
    if ($target == 0) {
	return NULL;
    }

    vector<OmMSetItem>::const_iterator i = $source.begin();
    while (i != $source.end()) {
        PyObject *t = PyTuple_New(3);

	PyTuple_SetItem(t, 0, PyInt_FromLong(i->did));
	PyTuple_SetItem(t, 1, PyFloat_FromDouble(i->wt));
	PyTuple_SetItem(t, 2, PyString_FromString(i->collapse_key.value.c_str()));

	PyList_Append($target, t);
        ++i;
    }
%}

%addmethods OmMSet {
    %readonly
    // access to the items array
    PyObject *items;

    // for comparison
    int __cmp__(const OmMSet &other) {
	if (self->mbound != other.mbound) {
	    return (self->mbound < other.mbound)? -1 : 1;
	} else if (self->max_possible != other.max_possible) {
	    return (self->max_possible < other.max_possible)? -1 : 1;
	} else if (self->items.size() != other.items.size()) {
	    return (self->items.size() < other.items.size())? -1 : 1;
	}

	for (int i=0; i<self->items.size(); ++i) {
	    if (self->items[i].wt != other.items[i].wt) {
		return (self->items[i].wt < other.items[i].wt)? -1 : 1;
	    } else if (self->items[i].did != other.items[i].did) {
		return (self->items[i].did < other.items[i].did)? -1 : 1;
	    }
	}
	return 0;
    }
    %readwrite
}

%apply LangSpecificListType items { PyObject *items }

%typemap(python, out) OmKey {
    $target = PyString_FromString(($source)->value.c_str());
    delete $source;
    $source = 0;
}

%typemap(python, out) OmData {
    $target = PyString_FromString(($source)->value.c_str());
    delete $source;
    $source = 0;
}

%addmethods OmESet {
    %readonly
    PyObject *items;
    %readwrite
}

%typemap(python, in) const query_batch &(OmBatchEnquire::query_batch v){
    if (!PyList_Check($source)) {
        PyErr_SetString(PyExc_TypeError, "expected list");
        return NULL;
    }
    int numitems = PyList_Size($source);
    for (int i=0; i<numitems; ++i) {
        PyObject *obj = PyList_GetItem($source, i);
	if (PyTuple_Check(obj) && PyTuple_Size(obj) == 6) {
	    query_desc mydesc;
	    try {
		OmQuery *qp = get_py_omquery(PyTuple_GetItem(obj, 0));
		if (!qp) return NULL;
		mydesc.query = *qp;
		mydesc.first = get_py_int(PyTuple_GetItem(obj, 1));
		mydesc.maxitems = get_py_int(PyTuple_GetItem(obj, 2));
		mydesc.omrset = get_py_omrset(PyTuple_GetItem(obj, 3));
		mydesc.moptions = get_py_ommatchoptions(PyTuple_GetItem(obj, 4));
		mydesc.mdecider = get_py_ommatchdecider(PyTuple_GetItem(obj, 5));
		v.push_back(mydesc);
	    } catch (OmPythonProblem &) {
		return NULL;
	    }
	} else {
	    PyErr_SetString(PyExc_TypeError,
			    "expected tuple");
	    return NULL;
	}
    }
    $target = &v;
}

%addmethods mset_batch {
    OmMSet __getitem__(int i) {
	if (i >= self->size()) {
	    throw OmRangeError("Index out of range");
	}
	return (*self)[i].value();
    }

    int __len__() {
	return self->size();
    }
}
