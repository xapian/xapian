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
#include <Python.h>

/* Override SWIG's standard GIL locking machinery - we want to avoid the
 * overhead of thread locking when the user's code isn't using threads. */
#define SWIG_PYTHON_NO_USE_GIL

       class XapianSWIG_Python_Thread_Block {
         bool status;
         PyGILState_STATE state;
       public:
         void end() { if (status) { PyGILState_Release(state); status = false;} }
         XapianSWIG_Python_Thread_Block() : status(PyEval_ThreadsInitialized()) { if (status) state = PyGILState_Ensure(); }
         ~XapianSWIG_Python_Thread_Block() { end(); }
       };
       class XapianSWIG_Python_Thread_Allow {
         bool status;
         PyThreadState *save;
       public:
         void end() { if (status) { PyEval_RestoreThread(save); status = false; }}
         XapianSWIG_Python_Thread_Allow() : status(PyEval_ThreadsInitialized()) { if (status) save = PyEval_SaveThread(); }
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
