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

#ifndef OM_HGUARD_OMERROR_H
#define OM_HGUARD_OMERROR_H

#include <string>
#include <stdexcept>

#include "omtypes.h"

class OmErrorHandler;

/// Base class for all errors reported
class OmError {
    friend OmErrorHandler;
    private:
	/// A message explaining the error.
        std::string msg;

	/// A message giving the context in which the error occurred.
	std::string context;

	/** The type of the error.
	 */
	std::string type;

	/** Whether the error has passed through a handler yet.
	 */
	bool has_been_handled;

	/// assignment operator private and unimplemented
	void operator=(const OmError &copyme);
    protected:
    	/** Constructors are protected, since they can only
	 *  be used by derived classes anyway.
	 */
        OmError(const std::string &msg_,
		const std::string &context_,
		const std::string &type_);

	OmError(const OmError &copyme)
		: msg(copyme.msg),
		  context(copyme.context),
		  type(copyme.type),
		  has_been_handled(copyme.has_been_handled) {}
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

	/** Get the context of the error.
	 */
	std::string get_context() const
	{
	    return context;
	}

        /// Instantiations of OmError (as opposed to subclasses) are forbidden
	virtual ~OmError() = 0;
};

inline OmError::~OmError() {}

#define DEFINE_ERROR_BASECLASS(a, b) \
class a : public b { \
    protected: \
	/** Constructor used by derived classes. */ \
	a(const std::string &msg_, \
	  const std::string &context_, \
	  const std::string &type_) : b(msg_, context_, type_) {}; \
}

#define DEFINE_ERROR_CLASS(a, b) \
class a : public b { \
    public: \
	/** Constructor used publically. */ \
	a(const std::string &msg_, \
	  const std::string &context_ = "") : b(msg_, context_, #a) {}; \
    protected: \
	/** Constructor used by derived classes. */ \
	a(const std::string &msg_, \
	  const std::string &context_, \
	  const std::string &type_) : b(msg_, context_, type_) {}; \
}

#include "omerrortypes.h"

#endif /* OM_HGUARD_OMERROR_H */
