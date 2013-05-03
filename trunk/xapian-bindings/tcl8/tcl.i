%module xapian
%{
/* tcl.i: SWIG interface file for the Tcl bindings
 *
 * Copyright (c) 2006,2007,2011,2012 Olly Betts
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

%include ../xapian-head.i

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

#define XAPIAN_MIXED_SUBQUERIES_BY_ITERATOR_TYPEMAP

%typemap(typecheck, precedence=500) (XapianSWIGQueryItor qbegin, XapianSWIGQueryItor qend) {
    int dummy;
    $1 = (Tcl_ListObjLength(interp, $input, &dummy) == TCL_OK);
    /* FIXME: if we add more array typemaps, we'll need to check the elements
     * of the array here to disambiguate. */
}

%{
class XapianSWIGQueryItor {
    Tcl_Interp *interp;

    Tcl_Obj ** items;

    int n;

  public:
    typedef std::random_access_iterator_tag iterator_category;
    typedef Xapian::Query value_type;
    typedef Xapian::termcount_diff difference_type;
    typedef Xapian::Query * pointer;
    typedef Xapian::Query & reference;

    XapianSWIGQueryItor()
	: n(0) { }

    XapianSWIGQueryItor(Tcl_Interp * interp_, Tcl_Obj **items_, int n_)
	: interp(interp_), items(items_), n(n_) { }

    XapianSWIGQueryItor & operator++() {
	++items;
	--n;
	return *this;
    }

    Xapian::Query operator*() const {
	Tcl_Obj * item = *items;
	Xapian::Query * subq = Xapian::get_tcl8_query(interp, item);
	if (subq)
	    return *subq;

	int len;
	const char *p = Tcl_GetStringFromObj(item, &len);
	return Xapian::Query(string(p, len));
    }

    bool operator==(const XapianSWIGQueryItor & o) {
	return n == o.n;
    }

    bool operator!=(const XapianSWIGQueryItor & o) {
	return !(*this == o);
    }

    difference_type operator-(const XapianSWIGQueryItor &o) const {
        // Note: n counts *DOWN*, so reverse subtract.
        return o.n - n;
    }
};

%}

%typemap(in) (XapianSWIGQueryItor qbegin, XapianSWIGQueryItor qend) {
    Tcl_Obj ** items;
    int numitems;
    if (Tcl_ListObjGetElements(interp, $input, &numitems, &items) != TCL_OK) {
	return TCL_ERROR;
    }
    $1 = XapianSWIGQueryItor(interp, items, numitems);
    // $2 is default initialised where SWIG declares it.
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

// Custom Tcl exception handling:

%{
static int XapianTclHandleError(Tcl_Interp * interp, const Xapian::Error &e) {
    Tcl_ResetResult(interp);
    Tcl_SetErrorCode(interp, "XAPIAN", e.get_type(), NULL);
    Tcl_AppendResult(interp, e.get_msg().c_str(), NULL);
    return TCL_ERROR;
}

static int XapianTclHandleError(Tcl_Interp * interp) {
    Tcl_ResetResult(interp);
    Tcl_SetErrorCode(interp, "XAPIAN ?", NULL);
    Tcl_AppendResult(interp, "Unknown Error", NULL);
    return TCL_ERROR;
}
%}

%exception {
    try {
	$function
    } catch (const Xapian::Error &e) {
	return XapianTclHandleError(interp, e);
    } catch (...) {
	return XapianTclHandleError(interp);
    }
}

%include ../xapian-headers.i

/* vim:set syntax=cpp: */
