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

%typemap(python, in) const vector<OmQuery *> *(vector<OmQuery *> v){
    if (!PyList_Check($source)) {
        PyErr_SetString(PyExc_TypeError, "expected list");
        return NULL;
    }
    int numitems = PyList_Size($source);
    for (int i=0; i<numitems; ++i) {
        PyObject *obj = PyList_GetItem($source, i);
	if (PyInstance_Check(obj)) {
	    PyObject *mythis = PyDict_GetItemString(((PyInstanceObject *)obj)
						    ->in_dict, "this");
	    OmQuery *subqp;
	    if (char *err = SWIG_GetPtr(PyString_AsString(mythis),
			    (void **)&subqp,
			    "_OmQuery_p")) {
		cerr << "obj.this: " << PyString_AsString(mythis) << endl;
		cerr << "Problem is: " << err << endl;
		PyErr_SetString(PyExc_ValueError,
				"OmQuery object invalid");
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

%{
PyObject *OmMSet_items_get(OmMSet *mset)
{
    PyObject *retval = PyList_New(0);
    if (retval == 0) {
	return NULL;
    }

    vector<OmMSetItem>::const_iterator i = mset->items.begin();
    while (i != mset->items.end()) {
        PyObject *t = PyTuple_New(3);

	PyTuple_SetItem(t, 0, PyInt_FromLong(i->did));
	PyTuple_SetItem(t, 1, PyFloat_FromDouble(i->wt));
	PyTuple_SetItem(t, 2, PyString_FromString(i->collapse_key.value.c_str()));

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
    PyObject *items;
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
