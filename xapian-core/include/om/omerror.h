/* omerror.h
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

#ifndef OM_HGUARD_ERROR_H
#define OM_HGUARD_ERROR_H

#include <string>
#include <stdexcept>

#include "omtypes.h"

/// Base class for all errors reported
class OmError {
    private:
	/// A message explaining the error.
        std::string msg;

	/** The type of the error.
	 */
	std::string type;

	/// assignment operator private and unimplemented
	void operator=(const OmError &copyme);
    protected:
    	/** Constructors are protected, since they can only
	 *  be used by derived classes anyway.
	 */
        OmError(const std::string &msg_, const std::string &type_);
	OmError(const OmError &copyme) : msg(copyme.msg), type(copyme.type) {}
    public:
	/** Return a message describing the error.
	 *  This is in a human readable form.
	 */
        std::string get_msg() const
        {
            return msg;
        }

	/** Return the type of the error.
	 */
	std::string get_type() const
	{
	    return type;
	}

        /// Instantiations of OmError (as opposed to subclasses) are forbidden
	virtual ~OmError() = 0;
};

inline OmError::~OmError() {}

#define DEFINE_ERROR_BASECLASS(a, b) \
class a : public b { \
    protected: \
	/** Constructor used by derived classes. */ \
	a(const std::string &msg_, const std::string &type_) : b(msg_, type_) {}; \
}

#define DEFINE_ERROR_CLASS(a, b) \
class a : public b { \
    public: \
	/** Constructor used publically. */ \
	a(const std::string &msg_) : b(msg_, #a) {}; \
    protected: \
	/** Constructor used by derived classes. */ \
	a(const std::string &msg_, const std::string &type_) : b(msg_, type_) {}; \
}

/** Base class for errors due to programming errors.
 *  An exception derived from OmLogicError is thrown when a misuse
 *  of the API is detected.
 */
DEFINE_ERROR_BASECLASS(OmLogicError, OmError);

/** Base class for errors due to run time problems.
 *  An exception derived from OmRuntimeError is thrown when an
 *  error is caused by problems with the data or environment rather
 *  than a programming mistake.
 */
DEFINE_ERROR_BASECLASS(OmRuntimeError, OmError);

/** Thrown if an internal consistency check fails.
 *  This represents a bug in Muscat. */
DEFINE_ERROR_CLASS(OmAssertionError, OmLogicError);

/** Thrown when an attempt to use an unimplemented feature is made. */
DEFINE_ERROR_CLASS(OmUnimplementedError, OmLogicError);

/** Thrown when an invalid argument is supplied to the API. */
DEFINE_ERROR_CLASS(OmInvalidArgumentError, OmLogicError);

/** Thrown when an attempt is made to access a document which is not in the
 *  database.  This could occur either due to a programming error, or
 *  because the database has changed since running the query. */
DEFINE_ERROR_CLASS(OmDocNotFoundError, OmRuntimeError);

/** thrown when an element is out of range. */
DEFINE_ERROR_CLASS(OmRangeError, OmRuntimeError);

/** thrown when really weird stuff happens.  If this is thrown something
 *  has gone badly wrong.
 */
DEFINE_ERROR_CLASS(OmInternalError, OmRuntimeError);

/** thrown for miscellaneous database errors. */
DEFINE_ERROR_CLASS(OmDatabaseError, OmRuntimeError);

/** thrown when there is a communications problem with
 *  a remote database.
 */
DEFINE_ERROR_CLASS(OmNetworkError, OmRuntimeError);

/** Thrown when a network timeout is exceeded 
 */
DEFINE_ERROR_CLASS(OmNetworkTimeoutError, OmNetworkError);

/** Thrown when opening a database fails. */
DEFINE_ERROR_CLASS(OmOpeningError, OmDatabaseError);

/** Thrown when gaining a lock on a database fails. */
DEFINE_ERROR_CLASS(OmDatabaseLockError, OmDatabaseError);

/** Thrown when trying to access invalid data. */
DEFINE_ERROR_CLASS(OmInvalidResultError, OmRuntimeError);

#endif /* OM_HGUARD_ERROR_H */
