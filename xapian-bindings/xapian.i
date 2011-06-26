%module(directors="1") xapian

%{
/* xapian.i: the Xapian scripting interface.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2005 James Aylett
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011 Olly Betts
 * Copyright 2007 Lemur Consulting Ltd
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */
%}

// In C#, we wrap ++ and -- as ++ and --.
#ifdef SWIGCSHARP
#define NEXT(RET, CLASS) CLASS & next() { return ++(*self); }
#define PREV(RET, CLASS) CLASS & prev() { return --(*self); }
#elif defined SWIGJAVA
#define NEXT(RET, CLASS) RET next() { return *(++(*self)); }
#define PREV(RET, CLASS) RET prev() { return *(--(*self)); }
#else
#define NEXT(RET, CLASS) void next() { ++(*self); }
#define PREV(RET, CLASS) void prev() { --(*self); }
#endif

#ifndef SWIGPYTHON
#ifdef SWIGCSHARP
%rename(Apply) operator();
#else
%rename(apply) operator();
#endif
#endif

%include xapian-headers.i

namespace Xapian {

// xapian/dbfactory.h

// Database factory functions:
#if !defined SWIGCSHARP && !defined SWIGJAVA
namespace Auto {
    Database open_stub(const string & file);
}

namespace Brass {
    %rename(brass_open) open;
    Database open(const std::string &dir);
/* SWIG Tcl wrappers don't call destructors for classes returned by factory
 * functions, so don't wrap them so users are forced to use the
 * WritableDatabase ctor instead. */
#ifndef SWIGTCL
    WritableDatabase open(const std::string &dir, int action, int block_size = 8192);
#endif
}

namespace Chert {
    %rename(chert_open) open;
    Database open(const std::string &dir);
/* SWIG Tcl wrappers don't call destructors for classes returned by factory
 * functions, so don't wrap them so users are forced to use the
 * WritableDatabase ctor instead. */
#ifndef SWIGTCL
    WritableDatabase open(const std::string &dir, int action, int block_size = 8192);
#endif
}

namespace InMemory {
    %rename(inmemory_open) open;
    WritableDatabase open();
}

namespace Remote {
    %rename(remote_open) open;
    %rename(remote_open_writable) open_writable;

    Database open(const std::string &host, unsigned int port, Xapian::timeout timeout, Xapian::timeout connect_timeout);
    Database open(const std::string &host, unsigned int port, Xapian::timeout timeout = 10000);

    WritableDatabase open_writable(const std::string &host, unsigned int port, Xapian::timeout timeout, Xapian::timeout connect_timeout);
    WritableDatabase open_writable(const std::string &host, unsigned int port, Xapian::timeout timeout = 10000);

    Database open(const std::string &program, const std::string &args, Xapian::timeout timeout = 10000);

    WritableDatabase open_writable(const std::string &program, const std::string &args, Xapian::timeout timeout = 10000);
}
#else
/* Lie to SWIG that Auto, etc are classes with static methods rather than
   namespaces so it wraps it as we want in C# and Java. */
class Auto {
  private:
    Auto();
    ~Auto();
  public:
    static
    Database open_stub(const string & file);
};

class Brass {
  private:
    Brass();
    ~Brass();
  public:
    static
    Database open(const std::string &dir);
    static
    WritableDatabase open(const std::string &dir, int action, int block_size = 8192);
};

class Chert {
  private:
    Chert();
    ~Chert();
  public:
    static
    Database open(const std::string &dir);
    static
    WritableDatabase open(const std::string &dir, int action, int block_size = 8192);
};

class InMemory {
  private:
    InMemory();
    ~InMemory();
  public:
    static
    WritableDatabase open();
};

class Remote {
  private:
    Remote();
    ~Remote();
  public:
    static
    Database open(const std::string &host, unsigned int port, Xapian::timeout timeout, Xapian::timeout connect_timeout);
    static
    Database open(const std::string &host, unsigned int port, Xapian::timeout timeout = 10000);

    static
    WritableDatabase open_writable(const std::string &host, unsigned int port, Xapian::timeout timeout, Xapian::timeout connect_timeout);
    static
    WritableDatabase open_writable(const std::string &host, unsigned int port, Xapian::timeout timeout = 10000);

    static
    Database open(const std::string &program, const std::string &args, Xapian::timeout timeout = 10000);

    static
    WritableDatabase open_writable(const std::string &program, const std::string &args, Xapian::timeout timeout = 10000);
};
#endif

}

// xapian/query.h:

#if !defined SWIGTCL && !defined SWIGLUA && !defined SWIGPHP
// FIXME: wrap MatchAll and MatchNothing for other languages (except for
// Python, Ruby, and Perl which wrap them in a different way)
%ignore Xapian::Query::MatchAll;
%ignore Xapian::Query::MatchNothing;
#endif

%ignore Xapian::Query::internal;
%ignore Xapian::Query::operator=;
%extend Xapian::Query {
#ifndef XAPIAN_MIXED_VECTOR_QUERY_INPUT_TYPEMAP
	    /* For some languages we handle strings in the vector<Query>
	     * case, so we don't need to wrap this ctor. */

	    /** Constructs a query from a vector of terms merged with the
	     *  specified operator. */
	    Query(Query::op op, const vector<string> & subqs, termcount param = 0) {
		return new Xapian::Query(op, subqs.begin(), subqs.end(), param);
	    }
#endif

	    /** Constructs a query from a vector of subqueries merged with the
	     *  specified operator. */
	    Query(Query::op op, const vector<Xapian::Query> & subqs, termcount param = 0) {
		return new Xapian::Query(op, subqs.begin(), subqs.end(), param);
	    }
}
%include <xapian/query.h>
