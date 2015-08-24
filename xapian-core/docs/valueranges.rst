============
Value Ranges
============

.. contents:: Table of contents

Introduction
============

The ``Xapian::ValueRangeProcessor`` was introduced in Xapian 1.0.0.  It
provides a powerful and flexible way to parse range queries in the users'
query string.

This document describes the ``Xapian::ValueRangeProcessor`` class and
its standard subclasses, how to create your own subclasses, and how
these classes are used with ``Xapian::QueryParser``.

``Xapian::ValueRangeProcessor`` is a virtual base class, so you need to
use a subclass of it.  ``Xapian::QueryParser`` maintains a list of
``Xapian::ValueRangeProcessor`` objects which it tries in order for
each range search in the query until one accepts it, or all have been
tried (in which case an error is reported).

The ``Xapian::StringValueRangeProcessor`` subclass supports setting a prefix or
suffix string which must be present for the range to be recognised by that
object, and ``Xapian::DateValueRangeProcessor`` and
``Xapian::NumberValueRangeProcessor`` are subclasses of this so also
support a prefix or suffix (since Xapian 1.1.2 - before this all there were
direct subclasses of ``Xapian::ValueRangeProcessor``, with only
``Xapian::NumberValueRangeProcessor`` supporting this).

So you can support multiple filters distinguished by a prefix or suffix.  For
example, if you want to support range filters on price and weight, you can do
that like this::

    Xapian::QueryParser qp;
    Xapian::NumberValueRangeProcessor price_proc(0, "$", true);
    Xapian::NumberValueRangeProcessor weight_proc(1, "kg", false);
    qp.add_valuerangeprocessor(&price_proc);
    qp.add_valuerangeprocessor(&weight_proc);

Then the user can enter queries like::

    laptop $300..800 ..1.5kg

A common way to use this feature is with a prefix string which is a "field
name" followed by a colon, for example::

    created:1/1/1999..1/1/2003

Each ``Xapian::ValueRangeProcessor`` is passed the start and end of the
range.  If it doesn't understand the range, it should return
``Xapian::BAD_VALUENO``.  If it does understand the range, it should return
the value number to use with ``Xapian::Query::OP_VALUE_RANGE`` and if it
wants to, it can modify the start and end values (to convert them to the
correct format so that for the string comparison which ``OP_VALUE_RANGE``
uses).

In Xapian 1.2.1 and later, ``Xapian::QueryParser`` supports open-ended
ranges - if the start of the range is empty, that means any value less than
the end, and similarly if the end is empty, that means any value greater
than the start.  The start and end can't both be empty.

StringValueRangeProcessor
=========================

This is the simplest of the standard subclasses.  It understands any range
passed (so it should always be the last ``ValueRangeProcessor``) and it
doesn't alter the range start or end.

For example, suppose you have stored author names in value number 4, and want
the user to be able to filter queries by specifying ranges of values such as::

    mars asimov..bradbury

To do this, you can use a ``StringValueRangeProcessor`` like so::

    Xapian::QueryParser qp;
    Xapian::StringValueRangeProcessor author_proc(4);
    qp.add_valuerangeprocessor(&author_proc);

The parsed query will use ``OP_VALUE_RANGE``, so ``query.get_description()``
would report::

    Xapian::Query(mars:(pos=1) FILTER (VALUE_RANGE 4 asimov bradbury)

The ``VALUE_RANGE`` subquery will only match documents where value 4 is
>= asimov and <= bradbury (using a string comparison).

DateValueRangeProcessor
=======================

This class allows you to implement date range searches.  As well as the value
number to search, you can tell it whether to prefer US-style month/day/year
or European-style day/month/year, and specify the epoch year to use for
interpreting 2 digit years (the default is day/month/year with an epoch of
1970).  The best choice of settings depends on the expectations of your users.
As these settings are only applied at search time, you can also easily offer
different versions of your search front-end with different settings if that is
useful.

For example, if your users are American and the dates present in your database
can extend a decade or so into the future, you might use something like this
which specifies to prefer US-style dates and that the epoch year is 1930 (so
02/01/29 is February 1st 2029 while 02/01/30 is February 1st 1930)::

    Xapian::QueryParser qp;
    Xapian::DateValueRangeProcessor date_proc(0, true, 1930);
    qp.add_valuerangeprocessor(&date_proc);

The dates are converted to the format YYYYMMDD, so the values you index also
need to also be in this format - for example, if ``doc_time`` is a ``time_t``::

    char buf[9];
    if (strftime(buf, sizeof(buf), "%Y%m%d", gmtime(&doc_time))) {
        doc.add_value(0, buf);
    }

NumberValueRangeProcessor
=========================

.. note:: This class had a design flaw in Xapian 1.0.0 and 1.0.1 - you should
   avoid using it with releases of Xapian earlier than 1.0.2.

This class allows you to implement numeric range searches.  The numbers used
may be any number which is representable as a double, but requires that the
stored values which the range is being applied have been converted to strings
at index time using the ``Xapian::sortable_serialise()`` method::

    Xapian::Document doc;
    doc.add_value(0, Xapian::sortable_serialise(price));

This method produces strings which will sort in numeric order, so you can use
it if you want to be able to sort based on the value in numeric order, too.

Custom subclasses
=================

You can easily create your own subclasses of ``Xapian::ValueRangeProcessor``.
Your subclass needs to implement a method
``Xapian::valueno operator()(std::string &begin, std::string &end)``
so for example you could implement a better version of the author range
described above which only matches ranges with a prefix (e.g.
``author:asimov..bradbury``) and lower-cases the names::

    struct AuthorValueRangeProcessor : public Xapian::StringValueRangeProcessor {
        AuthorValueRangeProcessor()
            : StringValueRangeProcessor(4, "author:", true) { }

        Xapian::valueno operator()(std::string &begin, std::string &end) {
            // Let the base class do the prefix check.
            if (StringValueRangeProcessor::operator()(begin, end) == BAD_VALUENO)
                return BAD_VALUENO;
            begin = Xapian::Unicode::tolower(begin);
            end = Xapian::Unicode::tolower(end);
            return valno;
        }
    };

If you want to support open-ended ranges, you need to handle begin or end
being empty suitably.  ``Xapian::QueryParser`` won't call your subclass
with *both* begin and end being empty.

Using Several ValueRangeProcessors
==================================

If you want to allow the user to specify different types of ranges, you can
specify multiple ``ValueRangeProcessor`` objects to use.  Just add them in
the order you want them to be checked::

    Xapian::QueryParser qp;
    AuthorValueRangeProcessor author_proc();
    qp.add_valuerangeprocessor(&author_proc);
    Xapian::DateValueRangeProcessor date_proc(0, false, 1930);
    qp.add_valuerangeprocessor(&date_proc);

And then you can parse queries such as
``mars author:Asimov..Bradbury 01/01/1960..31/12/1969`` successfully.
