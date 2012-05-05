%{
/* tcl8/except.i: Custom tcl8 exception handling.
 *
 * Copyright (c) 2006,2007,2011 Olly Betts
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

#include <exception>

static int XapianTclHandleError(Tcl_Interp * interp, const Xapian::Error &e) {
    Tcl_ResetResult(interp);
    Tcl_SetErrorCode(interp, "XAPIAN", e.get_type(), NULL);
    Tcl_AppendResult(interp, e.get_msg().c_str(), NULL);
    return TCL_ERROR;
}

static int XapianTclHandleError(Tcl_Interp * interp, const std::exception &e) {
    Tcl_ResetResult(interp);
    Tcl_SetErrorCode(interp, "std::exception", NULL);
    Tcl_AppendResult(interp, e.what(), NULL);
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
    } catch (const std::exception &e) {
	return XapianTclHandleError(interp, e);
    } catch (...) {
	return XapianTclHandleError(interp);
    }
}

/* vim:set syntax=cpp:set noexpandtab: */
