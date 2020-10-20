CGI parameters to Omega
=======================

In addition to the parameters listed here, all other parameters are
retained. Arbitrary CGI parameters may be read with the $cgi{PARAM}
and $cgilist{PARAM} OmegaScript commands.

Note that all CGI parameters are modified by Omega before they become
part of $cgilist{} - see the section at the end for details.

Main query parameters
---------------------

DB
	database name.  If the DB parameter is specified more than once, each
        value is used to allow searching over multiple databases.  Also, the
        value of each DB parameter may be a list of database names separated by
        "/".  If no DB parameters are specified, then the database name
        defaults to ``default``.  If you want to search over a combination
        of databases by default then you can make the ``default`` database a
        stub database file - see the "Overview" document in xapian-core for
        details of the format.

xDB
	database(s) used for last query (separated by / if appropriate).
	If the database(s) used change then relevance judgements are
	discarded and the first page of matches is shown.  If xDB is not set,
	the database is assumed not to have changed, which means if you only
	deal with one database you don't have to pass a useless extra parameter
	around.

DEFAULTOP
	default operator - values recognised ``AND``, ``and``, ``OR``, ``or``.
	As of version 1.3.0, the default is ``AND`` (previously it was ``OR``).
	If you want to implement "match any words", set ``DEFAULTOP=or``.

P
	query string to parse (may occur multiple times - if so, each will be
	parsed and the results combined with ``OP_AND``).

P.\ *PREFIX*
	like ``P``, but parsed with the default prefix set to *PREFIX*.  For
	example, ``P.A`` will search the author by default.

xP
	terms from the previous parsed query - used to decide if
	this is a fresh query (in which case relevance judgements are
	discarded and the first page of matches is shown), an extended query
	(in which case the first page of matches is shown), or an unchanged
	query.

ADD
	if present, any ``X`` parameters are appended to the value of the first
	non-empty ``P`` parameter, or used to build a query if there are no
	non-empty ``P`` parameters (used for topterms support when JavaScript
	isn't supported or is disabled).

X
	topterms to add to query (each term in a separate ``X`` parameter).  If
	``ADD`` is set, these will be appended to the value of the first
	non-empty ``P`` parameter, or used to build a query if there are no
	non-empty ``P`` parameters.

R
	relevant document(s) (multiple values separated by ".")

MORELIKE
        value is a numeric Xapian document id to return similar pages to, or a
        term name (which will be looked up and the document id of the first
        document it indexes will be used - this allows a MORELIKE query based
        on the unique id from an external database).

        Since Omega 1.4.18, MORELIKE queries are built with explicit ``OR``
        operators if ``DEFAULTOP`` isn't ``OR`` (which it isn't by default
        since 1.3.0).

        Also since Omega 1.4.18, ``MORELIKE`` can be specified multiple times
        to find more documents like a specified set of documents.  In earlier
        versions, only one of the values would be used in this case.

RAWSEARCH
	when set to non-zero value, this prevents TOPDOC being snapped to a
	multiple of HITSPERPAGE.  Normally we snap TOPDOC like this so that
	things work nicely if HITSPERPAGE is in a picker or on radio buttons.
	If we're postprocessing the output of omega and want variable sized
	pages, this is unhelpful.

MINHITS
	can be set to look for more matches than would otherwise be looked for
	to you can be sure how many more consecutive pages will definitely be
	needed to show results.  By default omega asks for one hit more than
	the last one displayed on this page (so we know for sure if there is a
	next page or not).  If MINHITS is set, we ask for at least MINHITS
	matches from the start of the current page - you can think of MINHITS
	as defaulting to (HITSPERPAGE + 1).

Filtering parameters
--------------------

B
        general boolean filter terms.

        See the `overview document <overview.html>`_ for details of how
        multiple `B` and `N` parameters are handled.

N
        negated general boolean filter terms (new in Omega 1.3.5 - older
        versions will just ignore any `N` parameters).

        See the `overview document <overview.html>`_ for details of how
        multiple `B` and `N` parameters are handled.

COLLAPSE
	value slot number to use for removing duplicate documents.
	Additional documents in the MSet with the same value will be
	removed from the MSet. $value{$cgi{COLLAPSE}} can be used to
	access the actual value for each hit.


START.\ *SLOT* END.\ *SLOT* SPAN.\ *SLOT*
        One or more of these parameters can be specified for each *SLOT* to
        perform value-based date range filtering.  A document must fall into
        all of the specified ranges to match.

        The values stored in the database in the specified *SLOT* need to be
        be in one of these formats with the format detected by looking at
        the length of the value bounds (each slot must use a single format,
        but different slots can use different formats):

        * YYYYMMDDHHMM (e.g. 200702142359)
        * YYYYMMDD (e.g. 20070214)
        * a raw 4 byte big-endian value representing a time_t (omindex adds
          the last modified time in value slot 0 in this format).

        `SPAN.`\ *SLOT* specifies the number of days either up to
        `END.`\ *SLOT* (if set), after `START.`\ *SLOT* (if set) or before
        today's date (if neither the start nor end are given) (if all three
        parameters are specified for the same *SLOT* then `START.`\ *SLOT*
        is ignored).

        If `SPAN.`\ *SLOT* is not specified:

        * `START.`\ *SLOT* specifies the start of the range in the
          format YYYY, YYYYMM, YYYYMMDD or YYYYMMDDHHMM.  Default is the start
          of time.
        * `END.`\ *SLOT* specifies the end of the range in the
          format YYYY, YYYYMM, YYYYMMDD or YYYYMMDDHHMM.  Default is the end of
          time.

        Added in Xapian 1.4.8 - older versions will just ignore these
        parameters.

DATEVALUE
        This is an older way to specify a value-based date range filter, which
        only allows one date range filter to be applied to each query.
        `DATEVALUE` specifies the value slot number to use.  The format of
        the values stored in this slot in the database must be in one of the
        formats described above (YYYYMMDDHHMM, YYYYMMDD or a raw 4 byte
        big-endian time_t).

        Don't mix `START.`\ *SLOT*, `END.`\ *SLOT* and/or `SPAN.`\ *SLOT* with
        `DATEVALUE` on the same slot number.

        If `DATEVALUE` isn't set then `START`, `END` and `SPAN` will perform
        date filtering using an older approach based on D-, M-, and Y-prefixed
        terms.  This approach can only filter to a granularity of one day, so
        only the `YYYYMMDD` part of `START` and `END` are used.  Support for
        `YYYY` and `YYYYMM` in `START` and `END` for term-based date filtering
        was added in Xapian 1.4.8 - in earlier versions this failed with an
        error.

        Also instead of `START`/`END` defaulting to the start and end of time,
        they instead default to 1st January 1970 and today's date respectively.
        The term-based date range filtering also includes a special `Dlatest`
        term, which allows flagging a document as always current.  There's no
        equivalent to this for value-based date range filters.

START END SPAN
        like `START.`\ *SLOT*, `END.`\ *SLOT* and `SPAN.`\ *SLOT* but for value
        slot `DATEVALUE`, or for term-based date range filtering if `DATEVALUE`
        isn't set.

xFILTERS
	used to spot when the filters have changed from the previous search.
	Set this to $html{$filters} in your query template ($filters is a
	compact serialisation of the currently set B filters, date-range
	filters, COLLAPSE, and DEFAULTOP).  If xFILTERS is unset, the filters
	are assumed not to have changed (unlike xP).  In Omega <= 1.2.21 and <=
	1.3.3 they were always assumed to have changed in the situation, which
	meant you couldn't ever go past page 1 if you failed to set xFILTERS
	in your template.  Now failing to set it means that the first page
	won't be forced in some cases where it probably should be.

THRESHOLD
	apply a percentage cut-off at the value given by this parameter
	(clipped to the range 0-100).

Reordering parameters
---------------------

SORT
	specifies one or more value slot numbers to order results by.  The
	comparison used is a string compare of the unsigned byte values.

	The format of this parameter's value is a `+` or `-` specifying the
	direction of the sort followed by an unsigned integer value slot
	number.  Normally `+` means an ascending sort (so the first result has
	the lowest value of the sort key) and `-` means a descending sort -
	however `SORTREVERSE` can change this (see below).

	The sort direction character was added in 1.3.5 - earlier versions
	defaulted to a descending sort (and for compatibility this is still
	the behaviour if you omit the `+` or `-`).

	Earlier versions also parsed the value as a signed integer and then
	cast it to unsigned, so beware of using updated templates with older
	versions.

	The ability to specify more than one value slot number was added
	in 1.4.1.  Multiple slot specifiers are separated by zero or more
	whitespace and/or commas - e.g. `SORT=+1-0+4`, `SORT=+1, -2`, etc.

SORTREVERSE
	if non-zero, reverses the sort order specified by `SORT`.  This
	parameter has no effect unless `SORT` is also specified.

SORTAFTER
	if non-zero, order results by relevance, only sorting by value to
	order values with the same relevance score.  This parameter has no
	effect unless SORT is also specified.

DOCIDORDER
	set the ordering used when a comparison ends up being by docid (i.e.
	two documents with equal relevance and/or values).  By default (if
	DOCIDORDER isn't set or is empty) this puts them in ASCENDING order
	(the lowest document id ranks highest).  If DOCIDORDER is specified
	and non-empty it can begin with "D" for DESCENDING order, "A" for
	ASCENDING order or any other character ("X" by convention) for
	DONT_CARE (the Xapian database backend will use whichever order is most
	efficient).  Any characters after the first are ignored.

Display parameters and navigation
---------------------------------

FMT
	name of page format to use (may not contain ``..``).

HITSPERPAGE
	hits per page (integer) - clipped to range 10-1000.

TOPDOC
	first document to display (snapped to multiple of HITSPERPAGE
	if RAWSEARCH is not set)

If a parameter named '<' or '>' exists, Omega will go to the previous
or next results page (based on the value of TOPDOC), respectively. If
not, and a parameter named '[' or '#' exists, it will jump to the page
number given by that parameter (trailing junk after the number is
ignored). (See the section below on modification of CGI parameters to
see how this works.)

This means that <input type='image' .../> form buttons can have names
of the form '[ 3 ]', which looks nice in lynx, for tooltips, and so
on. For text-only links, you really need to write out the entire GET
parameters and use a normal anchor.

Modification of CGI parameters
------------------------------

Omega does some special mangling of CGI parameter names which is intended
to help with using image buttons, and also to enable providing nicer "alt" text
in older browsers.

In the intervening decades HTML4 introduced the `alt` tag and CSS now provides
cleaner ways to handle image buttons, so this mangling isn't as useful as it
once was, but for now we've left it in place for compatibility.

Image Buttons
~~~~~~~~~~~~~

When the user clicks on an image button `<input type="image" name="PARAM">`,
the browser passes two CGI parameters `PARAM.x` and `PARAM.y` whose values
report the x and y coordinates within the image that were clicked.

Image buttons allow for prettier navigation within search results, but what
the browser passes is unhelpful so Omega does some special mangling of
parameters with a `.x` or `.y` suffix:

 * `PARAM.y` is silently dropped
 * `PARAM.x` is truncated to `PARAM`

Then:

 * if the parameter name contains a space or (since 1.4.4) a tab, the value
   becomes everything after the first space/tab and the original value is
   ignored. (e.g.: `[ 2 ].x=NNN` becomes `[=2 ]`).
 * if the parameter name doesn't contain a space or (since 1.4.4) a tab:
    * if the parameter name is entirely numeric, the name becomes `#` and the
      value becomes the parameter name. (e.g.: `2.x=NNN` becomes `#=2`)
    * otherwise, the value is replaced with the parameter name (e.g.:
      `>.x=NNN` becomes `>=>`)

Then general processing (as below) is applied.

General
~~~~~~~

For **ALL** CGI parameters, the name is truncated at the first space or (since
1.4.4) a tab. So `[ page two ]=2` becomes `[=2`.
