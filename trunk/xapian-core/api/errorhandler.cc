/* errorhandler.cc - Decide if a Xapian::Error exception should be ignored.
 *
 * Copyright (C) 2006,2007,2011 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "xapian/errorhandler.h"
#include "xapian/error.h"

Xapian::ErrorHandler::~ErrorHandler() { }

void
Xapian::ErrorHandler::operator()(Xapian::Error & error)
{
    if (!error.already_handled) {
	error.already_handled = true;
	if (handle_error(error))
	    return;
    }
    throw error;
}
