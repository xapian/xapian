%module(directors="1", moduleimport="from . import _xapian
from new import instancemethod as new_instancemethod") xapian
%{
/* python.i: SWIG interface file for the Python bindings
 *
 * Copyright (C) 2011,2012,2013,2014,2015,2016 Olly Betts
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

%pythonbegin %{
"""
Xapian is a highly adaptable toolkit which allows developers to easily
add advanced indexing and search facilities to their own
applications. It supports the Probabilistic Information Retrieval
model and also supports a rich set of boolean query operators.

In addition to the doc strings provided by this python library, you
may wish to look at the library's overall documentation, either
installed along with the bindings or online at
<https://xapian.org/docs/bindings/python/>, as well as the library's
documentation, possibly installed with the library or with its
development files, or again online at <https://xapian.org/docs/>.
"""
%}

/* These were deprecated before Python 3 support was released. */
#define XAPIAN_BINDINGS_SKIP_DEPRECATED_DB_FACTORIES

%begin %{
#include <config.h>
#include <Python.h>

/* Override SWIG's standard GIL locking machinery - we want to avoid the
 * overhead of thread locking when the user's code isn't using threads,
 * and to handle the GIL in a way which also works in sub-interpreters.
 */
#define SWIG_PYTHON_NO_USE_GIL

#ifdef THREAD_LOCAL

static THREAD_LOCAL PyThreadState * swig_pythreadstate = NULL;

inline void swig_pythreadstate_ensure_init() { }

inline PyThreadState * swig_pythreadstate_reset() {
    PyThreadState * v = swig_pythreadstate;
    if (v) swig_pythreadstate = NULL;
    return v;
}

inline PyThreadState * swig_pythreadstate_set(PyThreadState * v) {
    PyThreadState * old = swig_pythreadstate;
    swig_pythreadstate = v;
    return old;
}

#else

#include <pthread.h>

static pthread_key_t swig_pythreadstate_key;
static pthread_once_t swig_pythreadstate_key_once = PTHREAD_ONCE_INIT;

static void swig_pythreadstate_make_key()
{
    if (pthread_key_create(&swig_pythreadstate_key, NULL) != 0)
	Py_FatalError("pthread_key_create failed");
}

inline void swig_pythreadstate_ensure_init() {
    pthread_once(&swig_pythreadstate_key_once, swig_pythreadstate_make_key);
}

inline PyThreadState * swig_pythreadstate_reset() {
    PyThreadState * v = (PyThreadState*)pthread_getspecific(swig_pythreadstate_key);
    if (v) pthread_setspecific(swig_pythreadstate_key, NULL);
    return v;
}

inline PyThreadState* swig_pythreadstate_set(PyThreadState * v) {
    PyThreadState * old = (PyThreadState*)pthread_getspecific(swig_pythreadstate_key);
    pthread_setspecific(swig_pythreadstate_key, (void*)v);
    return old;
}

#endif

class XapianSWIG_Python_Thread_Block {
    bool status;
  public:
    XapianSWIG_Python_Thread_Block() : status(false) {
	if (PyEval_ThreadsInitialized()) {
	    swig_pythreadstate_ensure_init();
	    PyThreadState * ts = swig_pythreadstate_reset();
	    if (ts) {
		status = true;
		PyEval_RestoreThread(ts);
	    }
	}
    }
    void end() {
	if (status) {
	    if (swig_pythreadstate_set(PyEval_SaveThread()))
		Py_FatalError("swig_pythreadstate set in XapianSWIG_Python_Thread_Block::end()");
	    status = false;
	}
    }
    ~XapianSWIG_Python_Thread_Block() { end(); }
};

class XapianSWIG_Python_Thread_Allow {
    bool status;
  public:
    XapianSWIG_Python_Thread_Allow() : status(PyEval_ThreadsInitialized()) {
	if (status) {
	    swig_pythreadstate_ensure_init();
	    if (swig_pythreadstate_set(PyEval_SaveThread()))
		Py_FatalError("swig_pythreadstate set in XapianSWIG_Python_Thread_Allow ctor");
	}
    }
    void end() {
	if (status) {
	    PyThreadState * ts = swig_pythreadstate_reset();
	    if (!ts)
		Py_FatalError("swig_pythreadstate unset in XapianSWIG_Python_Thread_Block::end()");
	    PyEval_RestoreThread(ts);
	    status = false;
	}
    }
    ~XapianSWIG_Python_Thread_Allow() { end(); }
};

#define SWIG_PYTHON_THREAD_BEGIN_BLOCK   XapianSWIG_Python_Thread_Block _xapian_swig_thread_block
#define SWIG_PYTHON_THREAD_END_BLOCK     _xapian_swig_thread_block.end()
#define SWIG_PYTHON_THREAD_BEGIN_ALLOW   XapianSWIG_Python_Thread_Allow _xapian_swig_thread_allow
#define SWIG_PYTHON_THREAD_END_ALLOW     _xapian_swig_thread_allow.end()
%}

// Use SWIG directors for Python wrappers.
#define XAPIAN_SWIG_DIRECTORS

%include version.i
%include ../xapian-head.i

// Doccomments from Doxgyen-generated XML from C++ API docs.
%include doccomments.i

// Manually added exceptions for cases where the automatic comments aren't
// helpful, or are missing.
%include extracomments.i

%include util.i

// get_description() should return 'str' via the default typemap.
%typemap(out) std::string get_description() = std::string;
%typemap(out) std::string __str__() = std::string;

// All other std::string returns should map to 'bytes'.
%typemap(out) std::string %{
    $result = PyBytes_FromStringAndSize($1.data(), $1.size());
%}
%typemap(directorin) std::string, const std::string & %{
    $input = PyBytes_FromStringAndSize($1_name.data(), $1_name.size());
%}

// And const char * too.
%typemap(out) const char * %{
    $result = PyBytes_FromString($1);
%}
%typemap(directorin) const char * %{
    $input = PyBytes_FromString($1_name);
%}

// Make xapian.version_string() return a Unicode string.
%typemap(out) const char * version_string %{
    $result = PyUnicode_FromString($1);
%}

// Parameters where passing Unicode makes no sense.
%typemap(typecheck) const std::string & serialised %{
    $1 = PyBytes_Check($input) ? 1 : 0;
%}
%typemap(in) const std::string & serialised (std::string bytes) {
    char * p;
    Py_ssize_t len;
    if (PyBytes_AsStringAndSize($input, &p, &len) < 0) SWIG_fail;
    bytes.assign(p, len);
    $1 = &bytes;
}

#define XAPIAN_MIXED_SUBQUERIES_BY_ITERATOR_TYPEMAP

// Don't release the GIL for this method since we use Python C API calls to do
// the iteration.
%nothreadallow Xapian::Query::Query(op op_, XapianSWIGQueryItor qbegin, XapianSWIGQueryItor qend, Xapian::termcount parameter = 0);

%typemap(typecheck, precedence=500) (XapianSWIGQueryItor qbegin, XapianSWIGQueryItor qend) {
    // Checking for a sequence is enough to disambiguate currently.
    $1 = PySequence_Check($input);
}

%{
class XapianSWIGQueryItor {
    mutable PyObject * seq;

    int i;

    /// str_obj must be a bytes object
    Xapian::Query str_obj_to_query(PyObject * str_obj) const {
	char * p;
	Py_ssize_t len;
	(void)PyBytes_AsStringAndSize(str_obj, &p, &len);
	return Xapian::Query(string(p, len));
    }

  public:
    typedef std::random_access_iterator_tag iterator_category;
    typedef Xapian::Query value_type;
    typedef Xapian::termcount_diff difference_type;
    typedef Xapian::Query * pointer;
    typedef Xapian::Query & reference;

    XapianSWIGQueryItor() : seq(NULL), i(0) { }

    void begin(PyObject * seq_) {
	seq = seq_;
    }

    void end(PyObject * seq_) {
	i = PySequence_Fast_GET_SIZE(seq_);
    }

    void free_seq() {
	Py_CLEAR(seq);
    }

    XapianSWIGQueryItor & operator++() {
	++i;
	return *this;
    }

    Xapian::Query operator*() const {
	PyObject * obj = PySequence_Fast_GET_ITEM(seq, i);

	// Unicode object.
	if (PyUnicode_Check(obj)) {
	    PyObject *s = PyUnicode_EncodeUTF8(PyUnicode_AS_UNICODE(obj),
					       PyUnicode_GET_SIZE(obj),
					       "ignore");
	    if (!s) goto fail;
	    Xapian::Query result = str_obj_to_query(s);
	    Py_DECREF(s);
	    return result;
	}

	// String.
	if (PyBytes_Check(obj))
	    return str_obj_to_query(obj);

	// xapian.Query object (or unexpected object type).
	{
	    Xapian::Query * result_ptr = Xapian::get_py_query(obj);
	    if (result_ptr) return *result_ptr;
	}

    fail:
	throw Xapian::InvalidArgumentError("Expected Query object or string");
    }

    bool operator==(const XapianSWIGQueryItor & o) {
	return i == o.i;
    }

    bool operator!=(const XapianSWIGQueryItor & o) {
	return !(*this == o);
    }

    difference_type operator-(const XapianSWIGQueryItor &o) const {
        return i - o.i;
    }
};

%}

%typemap(in) (XapianSWIGQueryItor qbegin, XapianSWIGQueryItor qend) {
    PyObject * seq;
    seq = PySequence_Fast($input,
			  "expected sequence of Query objects and/or strings");
    if (!seq) SWIG_fail;
    $1.begin(seq);
    $2.end(seq);
}

%typemap(freearg) (XapianSWIGQueryItor qbegin, XapianSWIGQueryItor qend)
%{ $1.free_seq(); %}

%include except.i
%include ../xapian-headers.i
%include extra.i
