/* tcl8/util.i: custom tcl8 typemaps for xapian-bindings
 *
 * Copyright (c) 2006 Olly Betts
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

%{
namespace Xapian {
    Query *get_tcl8_query(Tcl_Interp *interp, Tcl_Obj *obj) {
	Query * retval = 0;
	if (SWIG_ConvertPtr(obj, (void **)&retval,
			    SWIGTYPE_p_Xapian__Query, 0) != TCL_OK) {
	    retval = 0;
	}
	return retval;
    }
}
%}

#define XAPIAN_MIXED_VECTOR_QUERY_INPUT_TYPEMAP
%typemap(typecheck, precedence=500) const vector<Xapian::Query> & {
    int dummy;
    $1 = (Tcl_ListObjLength(interp, $input, &dummy) == TCL_OK);
    /* FIXME: if we add more array typemaps, we'll need to check the elements
     * of the array here to disambiguate. */
}

%typemap(in) const vector<Xapian::Query> & (vector<Xapian::Query> v) {
    int numitems;
    Tcl_Obj ** items;
    if (Tcl_ListObjGetElements(interp, $input, &numitems, &items) != TCL_OK) {
	return TCL_ERROR;
    }
    v.reserve(numitems);
    while (numitems--) {
	Tcl_Obj * item = *items++;
	Xapian::Query * subq = Xapian::get_tcl8_query(interp, item);
	if (subq == NULL) {
	    int len;
	    const char *p = Tcl_GetStringFromObj(item, &len);
	    v.push_back(Xapian::Query(string(p, len)));
	} else {
	    v.push_back(*subq);
	}
    }
    $1 = &v;
}

#define XAPIAN_TERMITERATOR_PAIR_OUTPUT_TYPEMAP
%typemap(out) std::pair<Xapian::TermIterator, Xapian::TermIterator> {
    Tcl_Obj * list = Tcl_NewListObj(0, NULL);

    for (Xapian::TermIterator i = $1.first; i != $1.second; ++i) {
	Tcl_Obj * str = Tcl_NewStringObj((*i).data(), (*i).length());
	if (Tcl_ListObjAppendElement(interp, list, str) != TCL_OK)
	    return TCL_ERROR;
    }

    Tcl_SetObjResult(interp, list);
}

/* vim:set syntax=cpp:set noexpandtab: */
