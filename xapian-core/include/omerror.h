/* error.h
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

#ifndef _error_h_
#define _error_h_

#include <string>
#include <stdexcept>

#include "omtypes.h"

class OmError {
    private:
        string msg;
    public:
        OmError(const string &error_msg)
        {
            msg = error_msg;
        }
        string get_msg()
        {
            return msg;
        }
};

class RangeError : public OmError {
    public:
	RangeError(const string &msg) : OmError(msg) {};
};

class OpeningError : public OmError {
    public:
        OpeningError(const string &msg) : OmError(msg) {};
};

class AssertionFailed : public OmError {
    public:
        AssertionFailed(const string &msg) : OmError(msg + " - assertion failed") {};
};

#endif /* _error_h_ */
