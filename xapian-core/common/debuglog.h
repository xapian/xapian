/** @file debuglog.h
 * @brief Debug logging macros.
 */
/* Copyright (C) 2008,2009,2010,2011,2014,2015 Olly Betts
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

// In places where we include library code in non-library contexts, we can't
// have debug logging enabled, as the support functions aren't visible, so
// we define XAPIAN_REALLY_NO_DEBUG_LOG there.
#ifdef XAPIAN_REALLY_NO_DEBUG_LOG
# ifdef XAPIAN_DEBUG_LOG
#  undef XAPIAN_DEBUG_LOG
# endif
#endif

#ifdef XAPIAN_DEBUG_LOG

#include "output-internal.h"
#include "pretty.h"

#include <cstring>
#include <ostream>
#include <sstream>
#include <string>

/// A categorisation of debug log messages.
enum debuglog_categories {
    // Never wanted.
    DEBUGLOG_CATEGORY_NEVER = 0,

    /// Public API method and function calls.
    DEBUGLOG_CATEGORY_API = ('A' - '@'),

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

    /// Related to replication.
    DEBUGLOG_CATEGORY_REPLICA = ('C' - '@'),

    /// Spelling correction.
    DEBUGLOG_CATEGORY_SPELLING = ('S' - '@'),

    /// Uncategorised.
    DEBUGLOG_CATEGORY_UNKNOWN = ('U' - '@'),

    /// Weight calculations.
    DEBUGLOG_CATEGORY_WTCALC = ('W' - '@'),

    /// Query stuff.
    DEBUGLOG_CATEGORY_QUERY = ('Y' - '@'),

    /// Messages which are always logged.
    DEBUGLOG_CATEGORY_ALWAYS = 31
};

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

    /// The current indent level.
    int indent_level;

    /// Initialise categories_mask.
    void initialise_categories_mask();

  public:
    /// Constructor.
    DebugLogger()
	: categories_mask(1 << DEBUGLOG_CATEGORY_API), fd(-1), indent_level(0)
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

    /// Log message @a msg of category @a category.
    void log_line(debuglog_categories category, const std::string & msg);

    void indent() { ++indent_level; }

    void outdent() {
	if (indent_level) --indent_level;
    }
};

namespace Xapian {
    /** Dummy type for "no arguments".
     *
     *  We pull this into the global namespace, and overload operator<< so that
     *  writing it to a stream should generate no code.
     */
    typedef enum { NO_ARGS } NoArguments_;
}

inline std::ostream & operator<<(std::ostream &o, Xapian::NoArguments_) {
    return o;
}

using Xapian::NO_ARGS;

extern DebugLogger xapian_debuglogger_;

/** Unconditionally log message @a MSG of category @a CATEGORY. */
// Note that MSG can contain '<<' so we don't "protect" it with brackets.
#define LOGLINE_ALWAYS_(CATEGORY, MSG) do { \
    std::ostringstream xapian_debuglog_ostream_; \
    xapian_debuglog_ostream_ << MSG; \
    xapian_debuglogger_.log_line(CATEGORY, xapian_debuglog_ostream_.str()); \
} while (false)

/** Log message @a MSG of category @a CATEGORY. */
// Note that MSG can contain '<<' so we don't "protect" it with brackets.
#define LOGLINE_(CATEGORY, MSG) do { \
    debuglog_categories xapian_debuglog_category_ = (CATEGORY); \
    if (xapian_debuglogger_.is_category_wanted(xapian_debuglog_category_)) { \
	LOGLINE_ALWAYS_(xapian_debuglog_category_, MSG); \
    } \
} while (false)

/** Helper class for debug logging of functions and methods.
 *
 *  We instantiate a DebugLogFunc object at the start of each logged function
 *  and method.  DebugLogFunc's constructor logs the parameters passed, the
 *  RETURN() macro sets the return value as a string, and DebugLogFunc's
 *  destructor logs this string.  If an exception is thrown during the method
 *  and causes it to exit, DebugLogFunc's destructor detects and logs this
 *  fact.
 */
class DebugLogFunc {
    /// This pointer (or 0 if this is a static method or a non-class function).
    const void * this_ptr;

    /// The category of log message to use for this function/method.
    debuglog_categories category;

    /// Function/method name.
    std::string func;

    /// Was an uncaught exception active when we entered this function?
    bool uncaught_exception;

  public:
    /// Constructor called when logging for a "normal" method or function.
    DebugLogFunc(const void * this_ptr_, debuglog_categories category_,
		 const char * return_type, const char * func_name,
		 const std::string & params)
	: this_ptr(this_ptr_), category(category_),
	  uncaught_exception(std::uncaught_exception())
    {
	if (is_category_wanted()) {
	    func.assign(return_type);
	    func += ' ';
	    func += func_name;
	    func += '(';
	    func += params;
	    func += ')';
	    LOGLINE_ALWAYS_(category, '[' << this_ptr << "] " << func);
	    xapian_debuglogger_.indent();
	}
    }

    /// Log the returned value.
    void log_return_value(const std::string & return_value) {
	xapian_debuglogger_.outdent();
	LOGLINE_(category, '[' << this_ptr << "] " << func << " returned: " <<
			   return_value);

	// Flag that we've logged the return already.
	category = DEBUGLOG_CATEGORY_NEVER;
    }

    /// Check if the current category of log message is wanted.
    bool is_category_wanted() const {
	return xapian_debuglogger_.is_category_wanted(category);
    }

    /** Destructor.
     *
     *  This logs that the function/method has returned if this is due to an
     *  exception or if the RETURN() macro hasn't been used.
     */
    ~DebugLogFunc() {
	if (!is_category_wanted()) return;
	xapian_debuglogger_.outdent();
	if (!uncaught_exception && std::uncaught_exception()) {
	    // An exception is causing the stack to be unwound.
	    LOGLINE_(category, '[' << this_ptr << "] " << func <<
			       " exited due to exception");
	} else {
	    LOGLINE_(category, '[' << this_ptr << "] " << func <<
			       " returned (not marked up for return logging)");
	}
    }
};

/** Helper class for debug logging of functions and methods returning void,
 *  and class constructors and destructors.
 *
 *  We instantiate a DebugLogFuncVoid object at the start of each logged
 *  function and method.  DebugLogFuncVoid's constructor logs the parameters
 *  passed, and DebugLogFunc's destructor logs that the function/method is
 *  returning.  If an exception is thrown during the method and causes it to
 *  exit, DebugLogFunc's destructor detects and logs this fact.
 */
class DebugLogFuncVoid {
    /// This pointer (or 0 if this is a static method or a non-class function).
    const void * this_ptr;

    /// The category of log message to use for this function/method.
    debuglog_categories category;

    /// Function/method name.
    std::string func;

    /// Was an uncaught exception active when we entered this function?
    bool uncaught_exception;

  public:
    /// Constructor called when logging for a "normal" method or function.
    DebugLogFuncVoid(const void * this_ptr_, debuglog_categories category_,
		     const char * func_name,
		     const std::string & params)
	: this_ptr(this_ptr_), category(category_),
	  uncaught_exception(std::uncaught_exception())
    {
	if (is_category_wanted()) {
	    func.assign("void ");
	    func += func_name;
	    func += '(';
	    func += params;
	    func += ')';
	    LOGLINE_ALWAYS_(category, '[' << this_ptr << "] " << func);
	    xapian_debuglogger_.indent();
	}
    }

    /// Constructor called when logging for a class constructor.
    DebugLogFuncVoid(const void * this_ptr_, debuglog_categories category_,
		     const std::string & params,
		     const char * class_name)
	: this_ptr(this_ptr_), category(category_),
	  uncaught_exception(std::uncaught_exception())
    {
	if (is_category_wanted()) {
	    func.assign(class_name);
	    func += "::";
	    // The ctor name is the last component if there are colons (e.g.
	    // for Query::Internal, the ctor is Internal.
	    const char * ctor_name = std::strrchr(class_name, ':');
	    if (ctor_name)
		++ctor_name;
	    else
		ctor_name = class_name;
	    func += ctor_name;
	    func += '(';
	    func += params;
	    func += ')';
	    LOGLINE_ALWAYS_(category, '[' << this_ptr << "] " << func);
	    xapian_debuglogger_.indent();
	}
    }

    /// Constructor called when logging for a class destructor.
    DebugLogFuncVoid(const void * this_ptr_, debuglog_categories category_,
		     const char * class_name)
	: this_ptr(this_ptr_), category(category_),
	  uncaught_exception(std::uncaught_exception())
    {
	if (is_category_wanted()) {
	    func.assign(class_name);
	    func += "::~";
	    // The dtor name is the last component if there are colons.
	    const char * dtor_name = std::strrchr(class_name, ':');
	    if (dtor_name)
		++dtor_name;
	    else
		dtor_name = class_name;
	    func += dtor_name;
	    func += "()";
	    LOGLINE_(category, '[' << this_ptr << "] " << func);
	    xapian_debuglogger_.indent();
	}
    }

    /// Check if the current category of log message is wanted.
    bool is_category_wanted() const {
	return xapian_debuglogger_.is_category_wanted(category);
    }

    /** Destructor.
     *
     *  This logs that the function/method has returned and whether this was
     *  due to an exception.
     */
    ~DebugLogFuncVoid() {
	if (!is_category_wanted()) return;
	xapian_debuglogger_.outdent();
	const char * reason;
	if (!uncaught_exception && std::uncaught_exception()) {
	    // An exception is causing the stack to be unwound.
	    reason = " exited due to exception";
	} else {
	    reason = " returned";
	}
	LOGLINE_ALWAYS_(category, '[' << this_ptr << "] " << func << reason);
    }
};

#ifdef __GNUC__
// __attribute__((unused)) supported since at least GCC 2.95.3.
# define XAPIAN_UNUSED __attribute__((unused))
#else
# define XAPIAN_UNUSED
#endif

/// Log a call to a method returning non-void.
#define LOGCALL(CATEGORY, TYPE, FUNC, PARAMS) \
    typedef TYPE xapian_logcall_return_type_ XAPIAN_UNUSED; \
    std::string xapian_logcall_parameters_; \
    if (xapian_debuglogger_.is_category_wanted(DEBUGLOG_CATEGORY_##CATEGORY)) { \
	std::ostringstream xapian_logcall_ostream_; \
	PrettyOStream<std::ostringstream> xapian_logcall_stream_(xapian_logcall_ostream_); \
	xapian_logcall_stream_ << PARAMS; \
	xapian_logcall_parameters_ = xapian_logcall_ostream_.str(); \
    } \
    DebugLogFunc xapian_logcall_(static_cast<const void *>(this), DEBUGLOG_CATEGORY_##CATEGORY, #TYPE, FUNC, xapian_logcall_parameters_)

/// Log a call to a method returning void.
#define LOGCALL_VOID(CATEGORY, FUNC, PARAMS) \
    std::string xapian_logcall_parameters_; \
    if (xapian_debuglogger_.is_category_wanted(DEBUGLOG_CATEGORY_##CATEGORY)) { \
	std::ostringstream xapian_logcall_ostream_; \
	PrettyOStream<std::ostringstream> xapian_logcall_stream_(xapian_logcall_ostream_); \
	xapian_logcall_stream_ << PARAMS; \
	xapian_logcall_parameters_ = xapian_logcall_ostream_.str(); \
    } \
    DebugLogFuncVoid xapian_logcall_(static_cast<const void *>(this), DEBUGLOG_CATEGORY_##CATEGORY, FUNC, xapian_logcall_parameters_)

/// Log a constructor call.
#define LOGCALL_CTOR(CATEGORY, CLASS, PARAMS) \
    std::string xapian_logcall_parameters_; \
    if (xapian_debuglogger_.is_category_wanted(DEBUGLOG_CATEGORY_##CATEGORY)) { \
	std::ostringstream xapian_logcall_ostream_; \
	PrettyOStream<std::ostringstream> xapian_logcall_stream_(xapian_logcall_ostream_); \
	xapian_logcall_stream_ << PARAMS; \
	xapian_logcall_parameters_ = xapian_logcall_ostream_.str(); \
    } \
    DebugLogFuncVoid xapian_logcall_(static_cast<const void *>(this), DEBUGLOG_CATEGORY_##CATEGORY, xapian_logcall_parameters_, CLASS)

/// Log a destructor call.
#define LOGCALL_DTOR(CATEGORY, CLASS) \
    DebugLogFuncVoid xapian_logcall_(static_cast<const void *>(this), DEBUGLOG_CATEGORY_##CATEGORY, CLASS)

/// Log a call to a static method returning a non-void type.
#define LOGCALL_STATIC(CATEGORY, TYPE, FUNC, PARAMS) \
    typedef TYPE xapian_logcall_return_type_ XAPIAN_UNUSED; \
    std::string xapian_logcall_parameters_; \
    if (xapian_debuglogger_.is_category_wanted(DEBUGLOG_CATEGORY_##CATEGORY)) { \
	std::ostringstream xapian_logcall_ostream_; \
	PrettyOStream<std::ostringstream> xapian_logcall_stream_(xapian_logcall_ostream_); \
	xapian_logcall_stream_ << PARAMS; \
	xapian_logcall_parameters_ = xapian_logcall_ostream_.str(); \
    } \
    DebugLogFunc xapian_logcall_(0, DEBUGLOG_CATEGORY_##CATEGORY, #TYPE, FUNC, xapian_logcall_parameters_)

/// Log a call to a static method returning void.
#define LOGCALL_STATIC_VOID(CATEGORY, FUNC, PARAMS) \
    std::string xapian_logcall_parameters_; \
    if (xapian_debuglogger_.is_category_wanted(DEBUGLOG_CATEGORY_##CATEGORY)) { \
	std::ostringstream xapian_logcall_ostream_; \
	PrettyOStream<std::ostringstream> xapian_logcall_stream_(xapian_logcall_ostream_); \
	xapian_logcall_stream_ << PARAMS; \
	xapian_logcall_parameters_ = xapian_logcall_ostream_.str(); \
    } \
    DebugLogFuncVoid xapian_logcall_(0, DEBUGLOG_CATEGORY_##CATEGORY, FUNC, xapian_logcall_parameters_)

/// Log returning a value.
#define RETURN(A) do { \
    xapian_logcall_return_type_ xapian_logcall_return_ = A; \
    if (xapian_logcall_.is_category_wanted()) { \
	std::ostringstream xapian_logcall_ostream_; \
	PrettyOStream<std::ostringstream> xapian_logcall_stream_(xapian_logcall_ostream_); \
	xapian_logcall_stream_ << xapian_logcall_return_; \
	xapian_logcall_.log_return_value(xapian_logcall_ostream_.str()); \
    } \
    return xapian_logcall_return_; \
} while (false)

/** Log message @a b of category @a a.
 *
 *  The message is logged on a line by itself.  To keep the debug log readable,
 *  it shouldn't have a trailing '\n', or contain an embedded '\n'.
 */
#define LOGLINE(a,b) LOGLINE_(DEBUGLOG_CATEGORY_##a, b)

/** Log the value of variable or expression @a b. */
#define LOGVALUE(a,b) LOGLINE_(DEBUGLOG_CATEGORY_##a, #b" = " << b)

#else

#define LOGCALL(CATEGORY, TYPE, FUNC, PARAMS) (void)0
#define LOGCALL_VOID(CATEGORY, FUNC, PARAMS) (void)0
#define LOGCALL_CTOR(CATEGORY, CLASS, PARAMS) (void)0
#define LOGCALL_DTOR(CATEGORY, CLASS) (void)0
#define LOGCALL_STATIC(CATEGORY, TYPE, FUNC, PARAMS) (void)0
#define LOGCALL_STATIC_VOID(CATEGORY, FUNC, PARAMS) (void)0
#define RETURN(A) return A
#define LOGLINE(a,b) (void)0
#define LOGVALUE(a,b) (void)0

#endif

#endif // XAPIAN_INCLUDED_DEBUGLOG_H
