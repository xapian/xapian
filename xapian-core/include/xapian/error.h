/** @file error.h
 *  @brief Hierarchy of classes which Xapian can throw as exceptions.
 */
/* Copyright (C) 2003,2004,2006 Olly Betts
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

#ifndef XAPIAN_INCLUDED_ERROR_H
#define XAPIAN_INCLUDED_ERROR_H

#include <string>

namespace Xapian {

class ErrorHandler;

/** All exceptions thrown by Xapian are subclasses of Xapian::Error.
 *
 *  This class can not be instantiated directly - instead a subclass should
 *  be used.
 */
class Error {
    // ErrorHandler needs to be able to access Error::already_handled.
    friend class ErrorHandler;

    /// Message giving details of the error, intended for human consumption.
    std::string msg;

    /** Optional context information.
     *
     *  This context is intended for use by Xapian::ErrorHandler (for example
     *  so it can know which remote server is unreliable and report the problem
     *  and remove that server from those being searched).  But it's typically
     *  a plain-text string, and so also fit for human consumption.
     */
    std::string context;

    /// The type of this error (e.g. DocNotFoundError.)
    const char * type;

    /** Optional value of 'errno' associated with this error.
     *
     *  If no value is associated, this member variable will be 0.
     *
     *  NB We don't just call this member "errno" to avoid problems on
     *  platforms where errno is a preprocessor macro.
     */
    int my_errno;

    /// True if this error has already been passed to an ErrorHandler.
    bool already_handled;

    /// Don't allow assignment of the base class.
    void operator=(const Error &o);

  protected:
    /** @private @internal
     *  @brief Constructor for use by constructors of derived classes.
     */
    Error(const std::string &msg_, const std::string &context_,
	  const char * type_, int errno_)
	: msg(msg_), context(context_), type(type_), my_errno(errno_),
	  already_handled(false) { }

  public:
    /// The type of this error (e.g. "DocNotFoundError".)
    std::string get_type() const { return std::string(type); }

    /// Message giving details of the error, intended for human consumption.
    const std::string & get_msg() const { return msg; }

    /** Optional context information.
     *
     *  This context is intended for use by Xapian::ErrorHandler (for example
     *  so it can know which remote server is unreliable and report the problem
     *  and remove that server from those being searched).  But it's typically
     *  a plain-text string, and so also fit for human consumption.
     */
    const std::string & get_context() const { return context; }

    /** Optional value of 'errno' associated with this error.
     *
     *  If no 'errno' value is associated, returns 0.
     */
    int get_errno() const { return my_errno; }
};

/* This hook is needed to allow SWIG generated-bindings to use GCC's
 * visibility support.  This can go away once we add GCC visibility support
 * to the main library.
 */
#if defined(SWIGEXPORT) && defined(__GNUC__) && (__GNUC__ >= 4)
# define XAPIAN_EXCEPTION_EXPORT SWIGEXPORT
#else
# define XAPIAN_EXCEPTION_EXPORT
#endif

/// @Internal Macro to define an abstract Xapian::Error subclass.
#define XAPIAN_DEFINE_ERROR_BASECLASS(CLASS, PARENT) \
class XAPIAN_EXCEPTION_EXPORT CLASS : public PARENT { \
  protected: \
    /** @Internal Constructor for use by constructors of derived classes. */ \
    CLASS(const std::string &msg_, const std::string &context_, \
	  const char * type_, int errno_) \
	: PARENT(msg_, context_, type_, errno_) {} \
}

/// @Internal Macro to define a concrete Xapian::Error subclass.
#define XAPIAN_DEFINE_ERROR_CLASS(CLASS, PARENT) \
class XAPIAN_EXCEPTION_EXPORT CLASS : public PARENT { \
  public: \
    /** General purpose constructor. */ \
    CLASS(const std::string &msg_, const std::string &context_ = "", \
	  int errno_ = 0) \
	: PARENT(msg_, context_, #CLASS, errno_) {} \
    /** Construct from message and errno value. */ \
    CLASS(const std::string &msg_, int errno_) \
	: PARENT(msg_, "", #CLASS, errno_) {} \
  protected: \
    /** @Internal Constructor for use by constructors of derived classes. */ \
    CLASS(const std::string &msg_, const std::string &context_, \
	  const char * type_, int errno_) \
	: PARENT(msg_, context_, type_, errno_) {} \
}

#include <xapian/errortypes.h>

#undef XAPIAN_DEFINE_ERROR_BASECLASS
#undef XAPIAN_DEFINE_ERROR_CLASS
#undef XAPIAN_EXCEPTION_EXPORT

}

#endif /* XAPIAN_INCLUDED_ERROR_H */
