============
Value Ranges
============

.. contents:: Table of contents

Introduction
============

The ``Xapian::RangeProcessor`` class was introduced in Xapian 1.3.6, and
provides a powerful and flexible way to parse range queries in the users'
query string.  It's a replacement for the older ``Xapian::ValueRangeProcessor``
class (which dates back to Xapian 1.0.0).

This document describes the ``Xapian::RangeProcessor`` class and
its standard subclasses, how to create your own subclasses, and how
these classes are used with ``Xapian::QueryParser``.

``Xapian::RangeProcessor`` itself supports parsing string ranges, optionally
only recognising ranges with a specified prefix or suffix.  There are
standard subclasses supporting ranges of dates (``Xapian::DateRangeProcessor``)
and of numbers (``Xapian::NumberRangeProcessor``).  User subclasses
can support custom range types.

``Xapian::QueryParser`` maintains a list of ``Xapian::RangeProcessor`` objects
which it tries in order for each range specified in the query until one accepts
it, or all have been tried (in which case an error is reported).

So you can support multiple filters distinguished by a prefix or suffix.  For
example, if you want to support range filters on price and weight, you can do
that like this::

    Xapian::QueryParser qp;
    Xapian::NumberRangeProcessor price_proc(0, "$");
    Xapian::NumberRangeProcessor weight_proc(1, "kg", Xapian::RP_SUFFIX);
    qp.add_rangeprocessor(&price_proc);
    qp.add_rangeprocessor(&weight_proc);

Then the user can enter queries like::

    laptop $300..800 ..1.5kg

A common way to use this feature is with a prefix string which is a "field
name" followed by a colon, for example::

    created:1/1/1999..1/1/2003

Each ``Xapian::RangeProcessor`` is passed the start and end of the
range.  If it doesn't understand the range, it should
``Xapian::Query(Xapian::Query::OP_INVALID)``.  If it does understand the range,
it should return a query object matching the range (which will often use query
operator ``Xapian::Query::OP_VALUE_RANGE`` but can be any query).

In Xapian 1.2.1 and later, ``Xapian::QueryParser`` supports open-ended
ranges - if the start of the range is empty, that means any value less than
the end, and similarly if the end is empty, that means any value greater
than the start.  The start and end can't both be empty.

RangeProcessor
==============

This understands any range passed which has the specified prefix or suffix.
If no prefix or suffix is specified it will match any range (so it's not
useful to specify further ``RangeProcessor`` objects after such an object
as they can't match).

For example, suppose you have stored author names in value number 4, and want
the user to be able to filter queries by specifying ranges of values such as::

    mars asimov..bradbury

To do this, you can use a ``RangeProcessor`` like so::

    Xapian::QueryParser qp;
    Xapian::RangeProcessor author_proc(4);
    qp.add_rangeprocessor(&author_proc);

The parsed query will use ``OP_VALUE_RANGE``, so ``query.get_description()``
would report::

    Xapian::Query(mars:(pos=1) FILTER (VALUE_RANGE 4 asimov bradbury)

The ``VALUE_RANGE`` subquery will only match documents where value 4 is
>= asimov and <= bradbury (using a string comparison).

DateRangeProcessor
==================

This class allows you to implement date range searches.  As well as the value
number to search, you can tell it whether to prefer US-style month/day/year
or European-style day/month/year (by using the ``Xapian::RP_DATE_PREFER_MDY``
flag), and specify the epoch year to use for interpreting 2 digit years (the
default is day/month/year with an epoch of 1970).  The best choice of settings
depends on the expectations of your users.  As these settings are only applied
at search time, you can also easily offer different versions of your search
front-end with different settings if that is useful.

For example, if your users are American and the dates present in your database
can extend a decade or so into the future, you might use something like this
which specifies to prefer US-style dates and that the epoch year is 1930 (so
02/01/29 is February 1st 2029 while 02/01/30 is February 1st 1930)::

    Xapian::QueryParser qp;
    Xapian::DateRangeProcessor date_proc(0, Xapian::RP_DATE_PREFER_MDY, 1930);
    qp.add_rangeprocessor(&date_proc);

The dates are converted to the format YYYYMMDD, so the values you index also
need to also be in this format - for example, if ``doc_time`` is a ``time_t``::

    char buf[9];
    if (strftime(buf, sizeof(buf), "%Y%m%d", gmtime(&doc_time))) {
        doc.add_value(0, buf);
    }

NumberRangeProcessor
====================

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

You can easily create your own subclasses of ``Xapian::RangeProcessor``.
Your subclass needs to implement a method
``Xapian::Query operator()(const std::string &begin, const std::string &end)``
so for example you could implement a better version of the author range
described above which only matches ranges with a prefix (e.g.
``author:asimov..bradbury``) and lower-cases the names::

    struct AuthorRangeProcessor : public Xapian::RangeProcessor {
        AuthorRangeProcessor() : RangeProcessor(4, "author:") { }

        Xapian::valueno operator()(const std::string& b, const std::string& e) {
            // Let the base class do the prefix check.
            return RangeProcessor::operator()(Xapian::Unicode::tolower(b),
                                              Xapian::Unicode::tolower(e));
        }
    };

If you want to support open-ended ranges, you need to handle begin or end
being empty suitably.  ``Xapian::QueryParser`` won't call your subclass
with *both* begin and end being empty.

Using Several RangeProcessors
=============================

If you want to allow the user to specify different types of ranges, you can
specify multiple ``RangeProcessor`` objects to use.  Just add them in
the order you want them to be checked::

    Xapian::QueryParser qp;
    AuthorRangeProcessor author_proc();
    qp.add_rangeprocessor(&author_proc);
    Xapian::DateRangeProcessor date_proc(0, 0, 1930);
    qp.add_rangeprocessor(&date_proc);

And then you can parse queries such as
``mars author:Asimov..Bradbury 01/01/1960..31/12/1969`` successfully.
