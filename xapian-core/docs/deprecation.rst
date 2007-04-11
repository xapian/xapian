===========
Deprecation
===========

.. warning:: (10 April 2007) This document is a draft written by Richard Boulton, and the policies described in it have not yet been discussed by the Xapian developers.  Do not rely on the information in it until the draft status is removed.

.. contents:: Table of contents

Introduction
============

Xapian's API is fairly stable and has been polished piece by piece over time,
but it still occasionally needs to be changed.  This may be because a new
feature has been implemented and the interface needs to allow access to it, but
it may also be required in order to polish a rough edge which has been missed
in earlier versions of Xapian, or simply to reflect an internal change which
requires a modification to the external interface.

We aim to make such changes in a way that allows developers to work against a
stable API, while avoiding the need for the Xapian developers to maintain too
many old historical interface artifacts.  This document describes the process
we use to deprecate old pieces of the API, lists parts of the API which are
currently marked as deprecated, and also describes parts of the API which have
been deprecated for some time, and are now removed from the Xapian library.

It is possible for functions, methods, constants or even whole classes to be
deprecated, but to save words this document will often use the word "function"
to refer to any of these types of interface items.


Deprecation Procedure
=====================

Deprecation markers
-------------------

At any particular point, some parts of the C++ API will be marked as
"deprecated".  This is indicated with the ``XAPIAN_DEPRECATED`` macro, which
will cause compilers with appropriate support (such as GCC 3.1 or later, and
MSVC 7.0 or later) to emit warning messages about the use of deprecated
functions at compile time.

If a function is marked with one of these markers, you should avoid using it in
new code, and should migrate your code to use a replacement when possible.  The
code comments for the function, or the list of deprecated functions at the end
of this file, will describe possible alternatives to the deprecated function.

API and ABI compatibility
-------------------------

Within a series of releases with a given major and minor number, we try to
maintatain API and ABI forwards compatibility.   This means that an application
written and compiled against version `X.Y.a` of Xapian should work, without any
changes or need to recompile, with a later version `X.Y.b`, for all `a` <= `b`.

It is possible that a function may be marked as deprecated within a minor
release series - that is a function may be deprecated from version `X.Y.c`
onwards, where `c` is not zero.  The API and ABI will not be changed by this
deprecation, since the function will still be available in the API (though the
change may cause the compiler to emit new warnings at compile time).

In general, a function will be supported after being deprecated for an entire
series of releases with a given major and minor number.  For example, if a
function is deprecated in release `1.2.0`, it will be supported for the entire
`1.2.x` release series, and removed in release `1.3.0`.  If a function is
deprecated in release `1.2.5`, it will be supported for all remaining releases
in the `1.2.x` series, and also for all releases in the `1.3.x` series, and
will be removed in release `1.4.0`.

However, this rule may not be followed in all cases.  In particular, if a
function was marked as "temporary" in the documentation, it may be removed
faster (and possibly, without even passing through a stage of being
deprecated).

Deprecation in the bindings
---------------------------

When the Xapian API changes, the interface provided by the Xapian bindings will
usually change in step.  In addition, it is sometimes necessary to change the
way in which Xapian is wrapped by bindings - for example, to provide a better
convenience wrapper for iterators in Python.  Again, we aim to ensure that an
application written (and compiled, if the language being bound is a compiled
language) for version `X.Y.a` of Xapian should work without any changes or need
to recompile, with a later version `X.Y.b`, for all `a` <= `b`.

However, the bindings are a little less mature than the core C++ API, so we
don't intend to give the same guarantee that a function present and not
deprecated in version `X.Y.a` will work in all versions `X+1.Y.b`.  In other
words, we may remove functions which have been deprecated without waiting for
an entire release series to pass.

Any planned deprecations will be documented in the list of deprecations and
removed features at the end of this file.


How to avoid using deprecated functions
=======================================

We recommend taking the following steps to avoid depending on deprecated
functions when writing your applications:

 - If at all possible, test compile your project using a compiler which
   supports warnings about deprecated functions (such as GCC 3.1 or later), and
   check for such warnings.  Use the -Werror flag to GCC to ensure that you
   don't miss any of them.

 - Check the NEWS file for each new release for details of any new functions
   which are deprecated in the release.

 - Check the documentation comments, or the automatically extracted API
   documentation, for each function you use in your application.  This
   documentation will indicate functions which are deprecated, or planned for
   deprecation.

 - For applications which are not written in C++, there is currently no
   equivalent of the ``XAPIAN_DEPRECATED`` macro for the bindings, and thus
   there is no way for the bindings to give a warning if a deprecated function
   is used.  This would be a nice addition for those languages in which there
   is a reasonable way to give such warnings.  Until such a feature is
   implemented, all application writers using the bindings can do is to check
   the list of deprecated functions in each new release, or lookup the features
   you are using in the list at the end of this file.


Features currently marked for deprecation
=========================================

Native C++ API
--------------

=========== ========== ============================= =======================================
Deprecation Removal    Function name                 Upgrade suggestion
=========== ========== ============================= =======================================
0.9.0       1.0.0      Enquire::set_sort_forward()   Use Enquire::set_docid_order() instead.
0.9.0       1.0.0      Enquire::set_sorting()        Use Enquire::set_sort_by_*() instead.
0.9.0       1.0.0      Stem::stem_word(word)         Use Stem::operator()(word) instead.
0.8.4       1.0.0      Auto::open(path)              Use the Database(path) constructor instead.
0.8.4       1.0.0      Auto::open(path, action)      Use the WritableDatabase(path, action) constructor instead.
0.8.2       1.0.0      Query::is_empty()             Use Query::empty() instead.
0.8.0       1.0.0      Document::add_term_nopos()    Use Document::add_term() instead.
0.5.0       ?          Enquire::set_bias()           No replacement yet implemented.
=========== ========== ============================= =======================================


Bindings
--------

================= ========= =================== =======================================
Deprecated since  Language  Function name       Upgrade suggestion
================= ========= =================== =======================================
================= ========= =================== =======================================


Features removed from Xapian
============================

Native C++ API
--------------

================= ==================================== =======================================
Removed since     Function name                        Upgrade suggestion
================= ==================================== =======================================
1.0.0             QueryParser::set_stemming_options()  Use set_stemming_strategy() instead.
================= ==================================== =======================================


Bindings
--------

================= ========= =================== =======================================
Removed since     Language  Function name       Upgrade suggestion
================= ========= =================== =======================================
================= ========= =================== =======================================


Author
======

This document is copyright (C) 2007 Lemur Consulting Ltd, and was written by
Richard Boulton.
