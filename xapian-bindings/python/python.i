%module(directors="1") xapian
%{
/* python.i: SWIG interface file for the Python bindings
 *
 * Copyright (C) 2011 Olly Betts
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

%include ../xapian-head.i

// Doccomments from Doxgyen-generated XML from C++ API docs.
%include doccomments.i

// Manually added exceptions for cases where the automatic comments aren't
// helpful, or are missing.
%include extracomments.i

%include util.i
%include except.i
%include ../xapian.i
%include extra.i
