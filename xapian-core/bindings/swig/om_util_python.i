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
	temp = string(PyString_AsString($source));
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
#if 0  // for now don't bother checking for class.  Might be good to check
       // that we inherit from OmQueryPtr at some point, but probably not
       // necessary as long as obj.this is a valid OmQuery pointer.
	    PyClassObject *objtype = ((PyInstanceObject *)obj)->in_class;
	    string stype = PyString_AsString(objtype->cl_name);
	    if (stype != "OmQuery") {
	        string msg("expected OmQuery object, got: ");
		msg += stype;
		PyErr_SetString(PyExc_TypeError,
				msg.c_str());
		return NULL;
	    }
#endif
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
