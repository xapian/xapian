/* omerrorhandler.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#ifndef OM_HGUARD_OMERRORHANDLER_H
#define OM_HGUARD_OMERRORHANDLER_H

#include "omerror.h"

/// Base class for all errors reported
class OmErrorHandler {
    private:
	/// Assignment is not allowed
	void operator=(const OmErrorHandler &copyme);

	/// Copying is not allowed
	OmErrorHandler(const OmErrorHandler &copyme);

	/** Method called to handle an error.
	 *
	 *  This method must be implemented by a subclass of OmErrorHandler,
	 *  and is called when an error occurs.  It should return true if it
	 *  has handled the error and would like execution to continue as well
	 *  as possible, or false if it would like execution to stop and the
	 *  error to be propagated.
	 *
	 *  Even if the method returns true, execution may stop if the error
	 *  condition cannot be recovered from.
	 *
	 *  @param   error    The error which has occurred.
	 * 
	 *  @return  true to continue with operation, false to propagate the
	 *  error.
	 */
	virtual bool handle_error(OmError & error) = 0;

    public:
	/** Standard constructor
	 */
        OmErrorHandler() {}

        /** Standard destructor
	 */
	virtual ~OmErrorHandler() {};

	/** Method called to handle an error.
	 *
	 *  This method is called when an error occurs, and calls
	 *  handle_error() for user handlers to deal with the error.
	 *
	 *  @param   error    The error which has occurred.
	 */
	void operator() (OmError & error);
};

#endif /* OM_HGUARD_OMERRORHANDLER_H */
