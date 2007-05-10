===========
Deprecation
===========

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

It is possible for functions, methods, constants, types or even whole classes
to be deprecated, but to save words this document will often use the term
"features" to refer collectively to any of these types of interface items.


Deprecation Procedure
=====================

Deprecation markers
-------------------

At any particular point, some parts of the C++ API will be marked as
"deprecated".  This is indicated with the ``XAPIAN_DEPRECATED`` macro, which
will cause compilers with appropriate support (such as GCC 3.1 or later, and
MSVC 7.0 or later) to emit warning messages about the use of deprecated
features at compile time.

If a feature is marked with one of these markers, you should avoid using it in
new code, and should migrate your code to use a replacement when possible.  The
documentation comments for the feature, or the list at the end
of this file, will describe possible alternatives to the deprecated feature.

API and ABI compatibility
-------------------------

Within a series of releases with a given major and minor number, we try to
maintain API and ABI forwards compatibility.   This means that an application
written and compiled against version `X.Y.a` of Xapian should work, without any
changes or need to recompile, with a later version `X.Y.b`, for all `a` <= `b`.

It is possible that a feature may be marked as deprecated within a minor
release series - that is from version `X.Y.c`
onwards, where `c` is not zero.  The API and ABI will not be changed by this
deprecation, since the feature will still be available in the API (though the
change may cause the compiler to emit new warnings at compile time).

In general, a feature will be supported after being deprecated for an entire
series of releases with a given major and minor number.  For example, if a
feature is deprecated in release `1.2.0`, it will be supported for the entire
`1.2.x` release series, and removed in release `1.3.0`.  If a feature is
deprecated in release `1.2.5`, it will be supported for all remaining releases
in the `1.2.x` series, and also for all releases in the `1.3.x` series, and
will be removed in release `1.4.0`.

However, this rule may not be followed in all cases.  In particular, if a
feature was marked as "temporary" in the documentation, it may be removed
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
don't intend to give the same guarantee that a feature present and not
deprecated in version `X.Y.a` will work in all versions `X+1.Y.b`.  In other
words, we may remove features which have been deprecated without waiting for
an entire release series to pass.

Any planned deprecations will be documented in the list of deprecations and
removed features at the end of this file.


How to avoid using deprecated features
======================================

We recommend taking the following steps to avoid depending on deprecated
features when writing your applications:

 - If at all possible, test compile your project using a compiler which
   supports warnings about deprecated features (such as GCC 3.1 or later), and
   check for such warnings.  Use the -Werror flag to GCC to ensure that you
   don't miss any of them.

 - Check the NEWS file for each new release for details of any new features
   which are deprecated in the release.

 - Check the documentation comments, or the automatically extracted API
   documentation, for each feature you use in your application.  This
   documentation will indicate features which are deprecated, or planned for
   deprecation.

 - For applications which are not written in C++, there is currently no
   equivalent of the ``XAPIAN_DEPRECATED`` macro for the bindings, and thus
   there is no way for the bindings to give a warning if a deprecated feature
   is used.  This would be a nice addition for those languages in which there
   is a reasonable way to give such warnings.  Until such a feature is
   implemented, all application writers using the bindings can do is to check
   the list of deprecated features in each new release, or lookup the features
   they are using in the list at the end of this file.


Features currently marked for deprecation
=========================================

Native C++ API
--------------

+-----------------+----------------+-------------------------------+-------------------------------------------------------------------------------+
| **Deprecation** | **Removal**    | **Feature name**              | **Upgrade suggestion and comments**                                           |
+-----------------+----------------+-------------------------------+-------------------------------------------------------------------------------+
| 0.9.6           | 1.1.0          | xapian_version_string()       | Use version_string() instead.                                                 |
+-----------------+----------------+-------------------------------+-------------------------------------------------------------------------------+
| 0.9.6           | 1.1.0          | xapian_major_version()        | Use major_version() instead.                                                  |
+-----------------+----------------+-------------------------------+-------------------------------------------------------------------------------+
| 0.9.6           | 1.1.0          | xapian_minor_version()        | Use minor_version() instead.                                                  |
+-----------------+----------------+-------------------------------+-------------------------------------------------------------------------------+
| 0.9.6           | 1.1.0          | xapian_revision()             | Use revision() instead.                                                       |
+-----------------+----------------+-------------------------------+-------------------------------------------------------------------------------+
| 1.0.0           | 1.1.0          | Enquire::include_query_terms  | Use Enquire::INCLUDE_QUERY_TERMS instead.                                     |
+-----------------+----------------+-------------------------------+-------------------------------------------------------------------------------+
| 1.0.0           | 1.1.0          | Enquire::use_exact_termfreq   | Use Enquire::USE_EXACT_TERMFREQ instead.                                      |
+-----------------+----------------+-------------------------------+-------------------------------------------------------------------------------+
| 1.0.0           | 1.1.0          | Error::get_errno()            | Use Error::get_error_string() instead.                                        |
+-----------------+----------------+-------------------------------+-------------------------------------------------------------------------------+
| 1.0.0           | 1.1.0          | The Quartz backend            | Use the Flint backend instead.                                                |
+-----------------+----------------+-------------------------------+-------------------------------------------------------------------------------+
| 1.0.0           | 1.1.0          | Quartz::open()                | Use Flint::open() instead.                                                    |
+-----------------+----------------+-------------------------------+-------------------------------------------------------------------------------+
| 1.0.0           | 1.1.0          | quartzcheck                   | Use xapian-check instead.                                                     |
+-----------------+----------------+-------------------------------+-------------------------------------------------------------------------------+
| 1.0.0           | 1.1.0          | quartzcompact                 | Use xapian-compact instead.                                                   |
+-----------------+----------------+-------------------------------+-------------------------------------------------------------------------------+


Bindings
--------

+-----------------+-------------+----------------+-----------------------------+-------------------------------------------------------------------------------+
| **Deprecation** | **Removal** | **Language**   | **Feature name**            | **Upgrade suggestions and comments**                                          |
+-----------------+-------------+----------------+-----------------------------+-------------------------------------------------------------------------------+
| 0.9.6           | 1.1.0       | SWIG [#swig]_  | xapian_version_string()     | Use version_string() instead.                                                 |
+-----------------+-------------+----------------+-----------------------------+-------------------------------------------------------------------------------+
| 0.9.6           | 1.1.0       | SWIG [#swig]_  | xapian_major_version()      | Use major_version() instead.                                                  |
+-----------------+-------------+----------------+-----------------------------+-------------------------------------------------------------------------------+
| 0.9.6           | 1.1.0       | SWIG [#swig]_  | xapian_minor_version()      | Use minor_version() instead.                                                  |
+-----------------+-------------+----------------+-----------------------------+-------------------------------------------------------------------------------+
| 0.9.6           | 1.1.0       | SWIG [#swig]_  | xapian_revision()           | Use revision() instead.                                                       |
+-----------------+-------------+----------------+-----------------------------+-------------------------------------------------------------------------------+
| 1.0.0           | 1.1.0       | SWIG [#swig]_  | ESet::get_termname()        | Use ESet::get_term() instead.  This change is intended to bring the           |
|                 |             |                |                             | ESet iterators in line with other term iterators, which all support           |
|                 |             |                |                             | get_term() instead of get_termname()                                          |
+-----------------+-------------+----------------+-----------------------------+-------------------------------------------------------------------------------+
| 1.0.0           | 1.1.0       | Python         | get_description()           | All get_description() methods have been renamed to __str__(), so the normal   |
|                 |             |                |                             | python str() function can be used.                                            |
+-----------------+-------------+----------------+-----------------------------+-------------------------------------------------------------------------------+
| 1.0.0           | 1.1.0       | Python         | MSetItem.get_*()            | All these methods are deprecated, in faviour of properties.  To convert,      |
|                 |             |                |                             | just change msetitem.get_FOO() to msetitem.FOO                                |
+-----------------+-------------+----------------+-----------------------------+-------------------------------------------------------------------------------+
| 1.0.0           | 1.1.0       | Python         | Enquire.get_matching_terms  | Replaced by Enquire.matching_terms, for consistency with rest of Python API.  |
+-----------------+-------------+----------------+-----------------------------+-------------------------------------------------------------------------------+
| 1.0.0           | 1.1.0       | SWIG [#swig]_  | Error::get_errno()          | Use Error::get_error_string() instead.                                        |
+-----------------+-------------+----------------+-----------------------------+-------------------------------------------------------------------------------+
| 0.9.6           | 1.1.0       | SWIG [#swig2]_ | MSet::get_document_id()     | Use MSet::get_docid() instead.                                                |
+-----------------+-------------+----------------+-----------------------------+-------------------------------------------------------------------------------+
| 0.9.0           | 1.0.0       | SWIG [#swig]_  | Stem::stem_word(word)       | Use Stem::operator()(word) instead. [#callable]_                              |
+-----------------+-------------+----------------+-----------------------------+-------------------------------------------------------------------------------+

.. [#swig] This affects all swig generated bindings (currently: Python, PHP, Ruby, Tcl8 and CSharp)

.. [#swig2] This affects all swig generated bindings except those for Ruby, support for which was added after the function was deprecated in Xapian-core.

.. [#callable] Python handles this like C++.  Ruby renames it to 'call' (idiomatic Ruby).  PHP renames it to 'apply'.  CSharp to 'Apply' (delegates could probably be used to provide C++-like functor syntax, but that's effort and it seems debatable if it would actually be more natural to a C# programmer).  Tcl8 renames it to 'apply' - need to ask a Tcl type if that's the best solution.

Omega
-----

+-----------------+----------------+-------------------------------+-------------------------------------------------------------------------------+
| **Deprecation** | **Removal**    | **Feature name**              | **Upgrade suggestion and comments**                                           |
+-----------------+----------------+-------------------------------+-------------------------------------------------------------------------------+
| 0.9.5           | 1.1.0          | scriptindex index=nopos       | Use indexnopos instead.                                                       |
+-----------------+----------------+-------------------------------+-------------------------------------------------------------------------------+

Features removed from Xapian
============================

Native C++ API
--------------

+----------------+-------------------------------------+-----------------------------------------------------------------------------------------+
| **Removal**    | **Feature name**                    | **Upgrade suggestion and comments**                                                     |
+----------------+-------------------------------------+-----------------------------------------------------------------------------------------+
| 1.0.0          | QueryParser::set_stemming_options() | Use set_stemming_strategy() instead.                                                    |
|                |                                     | Use set_stemmer(), set_stemming_strategy() and/or set_stopper() instead.                |
|                |                                     |                                                                                         |
|                |                                     | - set_stemming_options("") becomes:                                                     |
|                |                                     |   set_stemming_strategy(Xapian::QueryParser::STEM_NONE)                                 |
|                |                                     | - set_stemming_options("none") becomes:                                                 |
|                |                                     |   set_stemming_strategy(Xapian::QueryParser::STEM_NONE)                                 |
|                |                                     | - set_stemming_options(LANG) becomes:                                                   |
|                |                                     |   set_stemmer(Xapian::Stem(LANG); set_stemming_strategy(Xapian::QueryParser::STEM_SOME) |
|                |                                     |                                                                                         |
|                |                                     | - set_stemming_options(LANG, false) becomes:                                            |
|                |                                     |   set_stemmer(Xapian::Stem(LANG); set_stemming_strategy(Xapian::QueryParser::STEM_SOME) |
|                |                                     |                                                                                         |
|                |                                     | - set_stemming_options(LANG, true) becomes:                                             |
|                |                                     |   set_stemmer(Xapian::Stem(LANG); set_stemming_strategy(Xapian::QueryParser::STEM_ALL)  |
|                |                                     |                                                                                         |
|                |                                     | If a third parameter is passed, set_stopper(PARAM3) and treat the first two             |
|                |                                     | parameters as above.                                                                    |
+----------------+-------------------------------------+-----------------------------------------------------------------------------------------+
| 1.0.0          | Enquire::set_sort_forward()         | Use Enquire::set_docid_order() instead:                                                 |
|                |                                     |                                                                                         |
|                |                                     |  - set_sort_forward(true) becomes set_docid_order(ASCENDING),                           |
|                |                                     |  - set_sort_forward(false) becomes set_docid_order(DESCENDING).                         |
+----------------+-------------------------------------+-----------------------------------------------------------------------------------------+
| 1.0.0          | Enquire::set_sorting()              | Use Enquire::set_sort_by_relevance(), Enquire::set_sort_by_value(), or                  |
|                |                                     | Enquire::set_sort_by_value_then_relevance() instead.                                    |
|                |                                     |                                                                                         |
|                |                                     |  - set_sorting(KEY, 1) becomes set_sort_by_value(KEY)                                   |
|                |                                     |  - set_sorting(KEY, 1, false) becomes set_sort_by_value(KEY)                            |
|                |                                     |  - set_sorting(KEY, 1, true) becomes set_sort_by_value_then_relevance(KEY)              |
|                |                                     |  - set_sorting(ANYTHING, 0) becomes set_sort_by_relevance()                             |
|                |                                     |  - set_sorting(Xapian::BAD_VALUENO, ANYTHING) becomes set_sort_by_relevance()           |
+----------------+-------------------------------------+-----------------------------------------------------------------------------------------+
| 1.0.0          | Stem::stem_word(word)               | Use Stem::operator()(word) instead.                                                     |
+----------------+-------------------------------------+-----------------------------------------------------------------------------------------+
| 1.0.0          | Auto::open(path)                    | Use the Database(path) constructor instead.                                             |
+----------------+-------------------------------------+-----------------------------------------------------------------------------------------+
| 1.0.0          | Auto::open(path, action)            | Use the WritableDatabase(path, action) constructor instead.                             |
+----------------+-------------------------------------+-----------------------------------------------------------------------------------------+
| 1.0.0          | Query::is_empty()                   | Use Query::empty() instead.                                                             |
+----------------+-------------------------------------+-----------------------------------------------------------------------------------------+
| 1.0.0          | Document::add_term_nopos()          | Use Document::add_term() instead.                                                       |
+----------------+-------------------------------------+-----------------------------------------------------------------------------------------+
| 1.0.0          | Enquire::set_bias()                 | No replacement yet implemented.                                                         |
+----------------+-------------------------------------+-----------------------------------------------------------------------------------------+
| 1.0.0          | ExpandDecider::operator()           | Return type is now ``bool`` not ``int``.                                                |
+----------------+-------------------------------------+-----------------------------------------------------------------------------------------+
| 1.0.0          | MatchDecider::operator()            | Return type is now ``bool`` not ``int``.                                                |
+----------------+-------------------------------------+-----------------------------------------------------------------------------------------+
| 1.0.0          | Error::get_type()                   | Return type is now ``const char *`` not ``std::string``.                                |
+----------------+-------------------------------------+-----------------------------------------------------------------------------------------+


Bindings
--------

+-------------+-----------------+-----------------------------+-------------------------------------------------------------------------------+
| **Removal** | **Language**    | **Feature name**            | **Upgrade suggestions and comments**                                          |
+-------------+-----------------+-----------------------------+-------------------------------------------------------------------------------+
| 1.0.0       | SWIG [#rswig]_  | Enquire::set_sort_forward() | Use Enquire::set_sort_forward() instead.                                      |
|             |                 |                             |                                                                               |
|             |                 |                             |  - set_sort_forward(true) becomes set_docid_order(ASCENDING),                 |
|             |                 |                             |  - set_sort_forward(false) becomes set_docid_order(DESCENDING).               |
+-------------+-----------------+-----------------------------+-------------------------------------------------------------------------------+
| 1.0.0       | SWIG [#rswig]_  | Enquire::set_sorting()      | Use Enquire::set_sort_by_relevance(), Enquire::set_sort_by_value(),           |
|             |                 |                             | or Enquire::set_sort_by_value_then_relevance() instead.                       |
|             |                 |                             |                                                                               |
|             |                 |                             |  - set_sorting(KEY, 1) becomes set_sort_by_value(KEY)                         |
|             |                 |                             |  - set_sorting(KEY, 1, false) becomes set_sort_by_value(KEY)                  |
|             |                 |                             |  - set_sorting(KEY, 1, true) becomes set_sort_by_value_then_relevance(KEY)    |
|             |                 |                             |  - set_sorting(ANYTHING, 0) becomes set_sort_by_relevance()                   |
|             |                 |                             |  - set_sorting(Xapian::BAD_VALUENO, ANYTHING) becomes set_sort_by_relevance() |
+-------------+-----------------+-----------------------------+-------------------------------------------------------------------------------+
| 1.0.0       | SWIG [#rswig]_  | Auto::open(path)            | Use the Database(path) constructor instead.                                   |
+-------------+-----------------+-----------------------------+-------------------------------------------------------------------------------+
| 1.0.0       | SWIG [#rswig]_  | Auto::open(path, action)    | Use the WritableDatabase(path, action) constructor instead.                   |
+-------------+-----------------+-----------------------------+-------------------------------------------------------------------------------+
| 1.0.0       | SWIG [#rswig3]_ | MSet::is_empty()            | Use MSet::empty() instead.                                                    |
+-------------+-----------------+-----------------------------+-------------------------------------------------------------------------------+
| 1.0.0       | SWIG [#rswig3]_ | ESet::is_empty()            | Use ESet::empty() instead.                                                    |
+-------------+-----------------+-----------------------------+-------------------------------------------------------------------------------+
| 1.0.0       | SWIG [#rswig3]_ | RSet::is_empty()            | Use RSet::empty() instead.                                                    |
+-------------+-----------------+-----------------------------+-------------------------------------------------------------------------------+
| 1.0.0       | SWIG [#rswig3]_ | Query::is_empty()           | Use Query::empty() instead.                                                   |
+-------------+-----------------+-----------------------------+-------------------------------------------------------------------------------+
| 1.0.0       | SWIG [#rswig]_  | Document::add_term_nopos()  | Use Document::add_term() instead.                                             |
+-------------+-----------------+-----------------------------+-------------------------------------------------------------------------------+
| 1.0.0       | CSharp          | ExpandDecider::Apply()      | Return type is now bool instead of int.                                       |
+-------------+-----------------+-----------------------------+-------------------------------------------------------------------------------+
| 1.0.0       | CSharp          | MatchDecider::Apply()       | Return type is now bool instead of int.                                       |
+-------------+-----------------+-----------------------------+-------------------------------------------------------------------------------+

.. [#rswig] This affects all swig generated bindings (currently: Python, PHP, Ruby, Tcl8 and CSharp)

.. [#rswig3] This affects all swig generated bindings except those for Ruby, which was added after the function was deprecated in Xapian-core, and PHP, where empty is a reserved word (and therefore, the method remains "is_empty").

Omega
-----

+----------------+-------------------------------------+-----------------------------------------------------------------------------------------+
| **Removal**    | **Feature name**                    | **Upgrade suggestion and comments**                                                     |
+----------------+-------------------------------------+-----------------------------------------------------------------------------------------+
| 1.0.0          | $freqs                              | $map{$queryterms,$_:&nbsp;$nice{$freq{$_}}}                                             |
+----------------+-------------------------------------+-----------------------------------------------------------------------------------------+
| 1.0.0          | scriptindex -u                      | -u was ignored for compatibility with 0.7.5 and earlier.                                |
+----------------+-------------------------------------+-----------------------------------------------------------------------------------------+
| 1.0.0          | scriptindex -q                      | -q was ignored for compatibility with 0.6.1 and earlier.                                |
+----------------+-------------------------------------+-----------------------------------------------------------------------------------------+

Author
======

This document is copyright (C) 2007 Lemur Consulting Ltd, and was written by
Richard Boulton.  Portions Copyright (C) 2007 Olly Betts.
