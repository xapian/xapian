/* omerror.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003 Olly Betts
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

#include <config.h>
#include <xapian/error.h>
#include <xapian/errorhandler.h>
#include "omdebug.h"

Xapian::Error::Error(const std::string &msg_,
		 const std::string &context_,
		 const std::string &type_,
		 int errno_value_)
	: msg(msg_), context(context_), type(type_),
	  errno_value(errno_value_), has_been_handled(false)
{
    DEBUGLINE(EXCEPTION, type << "(" << msg << ", " << context
	      << errno_value << ")");
}

void
Xapian::ErrorHandler::operator() (Xapian::Error & error)
{
    DEBUGLINE(EXCEPTION, 
	      "Invoking error handler for: " << error.get_type() << "(" <<
	      error.get_msg() << ", " << error.get_context() << ", " <<
	      error.get_errno() << ")");
    if (error.has_been_handled) {
	DEBUGLINE(EXCEPTION, "Error has been handled previously: rethrowing.");
	throw error;
    }
    error.has_been_handled = true;
    if (!handle_error(error)) {
	DEBUGLINE(EXCEPTION, "Error handler: Rethrowing error.");
	throw error;
    }
    DEBUGLINE(EXCEPTION, "Error handler: error dealt with, continuing execution if possible.");
}
