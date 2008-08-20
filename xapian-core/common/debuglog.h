/** @file debuglog.h
 * @brief Debug logging macros.
 */
/* Copyright (C) 2008 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#ifndef XAPIAN_INCLUDED_DEBUGLOG_H
#define XAPIAN_INCLUDED_DEBUGLOG_H

#ifdef XAPIAN_DEBUG_VERBOSE

#include "omtime.h"
#include "output.h"

#include <sstream>

/// A categorisation of debug log messages.
enum debuglog_categories {
    // Never wanted.
    DEBUGLOG_CATEGORY_NEVER = 0,

    /// Related to the public API.
    DEBUGLOG_CATEGORY_API = ('A' - '@'),

    /// Public API method calls.
    DEBUGLOG_CATEGORY_APICALL = ('C' - '@'),

    /// Related to database backends.
    DEBUGLOG_CATEGORY_DB = ('D' - '@'),

    /// Related to exception handling.
    DEBUGLOG_CATEGORY_EXCEPTION = ('X' - '@'),

    /// Query expansion.
    DEBUGLOG_CATEGORY_EXPAND = ('E' - '@'),

    /// Matcher.
    DEBUGLOG_CATEGORY_MATCH = ('M' - '@'),

    /// Debug output from the lemon-generated QueryParser code.
    DEBUGLOG_CATEGORY_QUERYPARSER = ('Q' - '@'),

    /// Related to the remote backend.
    DEBUGLOG_CATEGORY_REMOTE = ('R' - '@'),

    /// Spelling correction.
    DEBUGLOG_CATEGORY_SPELLING = ('S' - '@'),

    /// Uncategorised.
    DEBUGLOG_CATEGORY_UNKNOWN = ('U' - '@'),

    /// Weight calculations.
    DEBUGLOG_CATEGORY_WTCALC = ('W' - '@'),

    /// Messages which are always logged.
    DEBUGLOG_CATEGORY_ALWAYS = 31
};

// FIXME: STATIC_ASSERT(DEBUGLOG_CATEGORY_WTCALC < DEBUGLOG_CATEGORY_ALWAYS);

/// Class to actually do the logging.
class DebugLogger {
    /// Don't allow assignment.
    void operator=(const DebugLogger &);

    /// Don't allow copying.
    DebugLogger(const DebugLogger &);

    /// Mask bitmap of categories the user wants log messages for.
    unsigned int categories_mask;

    /// File descriptor for debug logging.
    int fd;

    /// The current indent, as a string of spaces.
    std::string indent_string;

    /// Initialise categories_mask.
    void initialise_categories_mask();

  public:
    /// Constructor.
    DebugLogger()
	: categories_mask(1 << DEBUGLOG_CATEGORY_APICALL), fd(-1),
	  indent_string(" ")
    { }

    /// Destructor.
    ~DebugLogger();

    /// Check if the user wants debug log messages of category @a category.
    bool is_category_wanted(debuglog_categories category) {
	// The argument will almost always be constant, so these inline checks
	// against DEBUGLOG_CATEGORY_ALWAYS and DEBUGLOG_CATEGORY_NEVER will
	// usually be optimised away, or become the only code path.
	if (category == DEBUGLOG_CATEGORY_ALWAYS) return true;
	if (category == DEBUGLOG_CATEGORY_NEVER) return false;
	if (fd == -1) initialise_categories_mask();
	return (categories_mask >> category) & 1;
    }

    /// Log message @msg of category @a category.
    void log_line(debuglog_categories category, const std::string & msg);

    void indent() { indent_string += ' '; }

    void outdent() {
	if (indent_string.size() > 1) {
	    indent_string.resize(indent_string.size() - 1);
	}
    }
};

extern DebugLogger xapian_debuglogger__;

/** Log message @a MSG of category @a CATEGORY. */
// Note that MSG can contain '<<' so we don't "protect" it with brackets.
#define LOGLINE_(CATEGORY, MSG) do { \
    if (xapian_debuglogger__.is_category_wanted(CATEGORY)) { \
	std::ostringstream xapian_debuglog_stream_; \
	xapian_debuglog_stream_ << MSG; \
	xapian_debuglogger__.log_line(CATEGORY, xapian_debuglog_stream_.str()); \
    } \
} while (false)

/** Helper class for debug logging of functions and methods.
 *
 *  We instantiate a DebugLogFunc object at the start of each logged function
 *  and method.  The constructor logs the parameters passed, the RETURN()
 *  macro sets the return value as a string, and the destructor logs this
 *  string.  If an exception is thrown, the destructor detects and logs this
 *  fact.
 */
class DebugLogFunc {
    /// This pointer (or 0 if this is a static method or a non-class function).
    const void * this_ptr;

    /// The category of log message to use for this function/method.
    debuglog_categories category;

    /// Function/method name.
    std::string func;

  public:
    /// Constructor called when logging for a class constructor or destructor.
    DebugLogFunc(const void * this_ptr_, debuglog_categories category_,
		 const char * func_name,
		 const std::string & params)
	: this_ptr(this_ptr_), category(category_)
    {
	if (xapian_debuglogger__.is_category_wanted(category)) {
	    func.assign(func_name);
	    func += '(';
	    func += params;
	    func += ')';
	    LOGLINE_(category, '[' << this_ptr << "] " << func);
	    xapian_debuglogger__.indent();
	}
    }

    /// Constructor called when logging for a "normal" method or function.
    DebugLogFunc(const void * this_ptr_, debuglog_categories category_,
		 const char * return_type, const char * func_name,
		 const std::string & params)
	: this_ptr(this_ptr_), category(category_)
    {
	if (xapian_debuglogger__.is_category_wanted(category)) {
	    func.assign(return_type);
	    func += ' ';
	    func += func_name;
	    func += '(';
	    func += params;
	    func += ')';
	    LOGLINE_(category, '[' << this_ptr << "] " << func);
	    xapian_debuglogger__.indent();
	}
    }

    /// Log the returned value.
    void log_return_value(const std::string & return_value) {
	xapian_debuglogger__.outdent();
	LOGLINE_(category, '[' << this_ptr << "] " << func << " returned: " <<
			   return_value);

	// Flag that we've logged the return already.
	category = DEBUGLOG_CATEGORY_NEVER;
    }

    /// Log the returned value.
    void log_return_void() {
	xapian_debuglogger__.outdent();
	LOGLINE_(category, '[' << this_ptr << "] " << func << " returned");

	// Flag that we've logged the return already.
	category = DEBUGLOG_CATEGORY_NEVER;
    }

    /** Destructor.
     *
     *  This logs that the function/method has returned and any return value.
     */
    ~DebugLogFunc() {
	if (!xapian_debuglogger__.is_category_wanted(category)) return;
	xapian_debuglogger__.outdent();
	if (std::uncaught_exception()) {
	    // An exception is causing the stack to be unwound.
	    LOGLINE_(category, '[' << this_ptr << "] " << func <<
			       " exited due to exception");
	} else {
	    LOGLINE_(category, '[' << this_ptr << "] " << func <<
			       " returned (not marked up for return logging)");
	}
    }
};

/// Log a call to a method returning non-void.
#define LOGCALL(CATEGORY, TYPE, FUNC, PARAMS) \
    typedef TYPE xapian_logcall_return_type_; \
    std::string xapian_logcall_parameters_; \
    if (xapian_debuglogger__.is_category_wanted(DEBUGLOG_CATEGORY_##CATEGORY)) { \
	std::ostringstream xapian_logcall_stream_; \
	xapian_logcall_stream_ << PARAMS; \
	xapian_logcall_parameters_ = xapian_logcall_stream_.str(); \
    } \
    DebugLogFunc xapian_logcall_(static_cast<const void *>(this), DEBUGLOG_CATEGORY_##CATEGORY, #TYPE, FUNC, xapian_logcall_parameters_)

/// Log a call to a method returning void.
#define LOGCALL_VOID(CATEGORY, FUNC, PARAMS) \
    std::string xapian_logcall_parameters_; \
    if (xapian_debuglogger__.is_category_wanted(DEBUGLOG_CATEGORY_##CATEGORY)) { \
	std::ostringstream xapian_logcall_stream_; \
	xapian_logcall_stream_ << PARAMS; \
	xapian_logcall_parameters_ = xapian_logcall_stream_.str(); \
    } \
    DebugLogFunc xapian_logcall_(static_cast<const void *>(this), DEBUGLOG_CATEGORY_##CATEGORY, FUNC, xapian_logcall_parameters_)

/// Log a constructor call.
#define LOGCALL_CTOR(CATEGORY, FUNC, PARAMS) \
    std::string xapian_logcall_parameters_; \
    if (xapian_debuglogger__.is_category_wanted(DEBUGLOG_CATEGORY_##CATEGORY)) { \
	std::ostringstream xapian_logcall_stream_; \
	xapian_logcall_stream_ << PARAMS; \
	xapian_logcall_parameters_ = xapian_logcall_stream_.str(); \
    } \
    DebugLogFunc xapian_logcall_(static_cast<const void *>(this), DEBUGLOG_CATEGORY_##CATEGORY, FUNC, xapian_logcall_parameters_)

/// Log a destructor call.
#define LOGCALL_DTOR(CATEGORY, FUNC) \
    DebugLogFunc xapian_logcall_(static_cast<const void *>(this), DEBUGLOG_CATEGORY_##CATEGORY, FUNC)

/// Log a call to a static method returning a non-void type.
#define LOGCALL_STATIC(CATEGORY, TYPE, FUNC, PARAMS) \
    typedef TYPE xapian_logcall_return_type_; \
    std::string xapian_logcall_parameters_; \
    if (xapian_debuglogger__.is_category_wanted(DEBUGLOG_CATEGORY_##CATEGORY)) { \
	std::ostringstream xapian_logcall_stream_; \
	xapian_logcall_stream_ << PARAMS; \
	xapian_logcall_parameters_ = xapian_logcall_stream_.str(); \
    } \
    DebugLogFunc xapian_logcall_(0, DEBUGLOG_CATEGORY_##CATEGORY, #TYPE, FUNC, xapian_logcall_parameters_)

/// Log a call to a static method returning void.
#define LOGCALL_STATIC_VOID(CATEGORY, FUNC, PARAMS) \
    std::string xapian_logcall_parameters_; \
    if (xapian_debuglogger__.is_category_wanted(DEBUGLOG_CATEGORY_##CATEGORY)) { \
	std::ostringstream xapian_logcall_stream_; \
	xapian_logcall_stream_ << PARAMS; \
	xapian_logcall_parameters_ = xapian_logcall_stream_.str(); \
    } \
    DebugLogFunc xapian_logcall_(0, DEBUGLOG_CATEGORY_##CATEGORY, FUNC, xapian_logcall_parameters_)

/// Log returning a value.
#define RETURN(A) do { \
    xapian_logcall_return_type_ xapian_logcall_return_ = A; \
    std::ostringstream xapian_logcall_stream_; \
    xapian_logcall_stream_ << xapian_logcall_return_; \
    xapian_logcall_.log_return_value(xapian_logcall_stream_.str()); \
    return xapian_logcall_return_; \
} while (false)

/// Log returning from a function with a void return type.
#define RETURN_VOID do { \
    xapian_logcall_.log_return_void(); \
    return; \
} while (false)

/** Log message @a b of category @a a.
 *
 *  The message is logged on a line by itself.  To keep the debug log readable,
 *  it shouldn't have a trailing '\n', or contain an embedded '\n'.
 */
#define LOGLINE(a,b) LOGLINE_(DEBUGLOG_CATEGORY_##a, b)

/** Log the value of variable or expression @a b. */
#define LOGVALUE(a,b) LOGLINE_(DEBUGLOG_CATEGORY_##a, #b" = " << b)

#endif

#endif // XAPIAN_INCLUDED_DEBUGLOG_H
