%{
/* om_util.i: the Open Muscat scripting interface helpers.
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
#undef list
#include "om/om.h"
#include <string>
%}
%include typemaps.i
%include exception.i
%{
#define OMSWIG_exception(type, msg) \
    SWIG_exception((type), const_cast<char *>((msg).c_str()))
%}

%except {
    try {
    	$function
    } catch (OmAssertionError &e) {
        OMSWIG_exception(SWIG_UnknownError,
		       string("OmAssertion: ") + e.get_msg());
    } catch (OmUnimplementedError &e) {
        OMSWIG_exception(SWIG_UnknownError,
		       string("OmUnimplemented: ") + e.get_msg());
    } catch (OmInvalidArgumentError &e) {
        OMSWIG_exception(SWIG_ValueError,e.get_msg());
    } catch (OmDocNotFoundError &e) {
        OMSWIG_exception(SWIG_RuntimeError,
		       string("OmDocNotFoundError: ") + e.get_msg());
    } catch (OmRangeError &e) {
        OMSWIG_exception(SWIG_IndexError,
		       string("OmRangeError: ") + e.get_msg());
    } catch (OmInternalError &e) {
        OMSWIG_exception(SWIG_UnknownError,
		       string("OmInternalError: ") + e.get_msg());
    } catch (OmDatabaseError &e) {
        OMSWIG_exception(SWIG_IOError,
		       string("OmDatabaseError: ") + e.get_msg());
    } catch (OmNetworkError &e) {
        OMSWIG_exception(SWIG_IOError,
		       string("OmNetworkError: ") + e.get_msg());
    } catch (OmInvalidResultError &e) {
        OMSWIG_exception(SWIG_ValueError,
		       string("OmInvalidResultError: ") + e.get_msg());
    } catch (...) {
        OMSWIG_exception(SWIG_UnknownError,
			 string("unknown error in Open Muscat"));
    }
}

%include "om_util_python.i"
%include "om_util_perl5.i"
