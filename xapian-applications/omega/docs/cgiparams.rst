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
	default operator - values recognised "AND", "and", "OR", "or".
	Default: OR.  If you want to implement "match all words", set
	DEFAULTOP=AND.

P
	probabilistic query (may occur multiple times).

xP
	terms from previous probabilistic query - used to decide if
	this is a fresh query (in which case relevance judgements are
	discarded and the first page of matches is shown), an extended query
	(in which case the first page of matches is shown), or an unchanged
	query.

ADD
	if present, any X parameters are added to the probabilistic
	query (used for topterms support when JavaScript isn't
	supported or is disabled).

X
	topterms to add to query.

R
	relevant document(s) (multiple values separated by ".")

MORELIKE
	value is a document id to return similar pages to, or a term name
	(which will be looked up and the document id of the first document it
	indexes will be used - this allows a MORELIKE query based on the
	unique id from an external database)

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
        general boolean filter terms.  See the `overview document
        <overview.html>`_ for details of how multiple B parameters are handled.

COLLAPSE
	value number to use for removing duplicate documents.
	Additional documents in the MSet with the same value will be
	removed from the MSet. $value{$cgi{COLLAPSE}} can be used to
	access the actual value for each hit.

DATEVALUE
	value number to use for date range filtering.  If this isn't set then
	date filtering will use the older approach based on D-, M-, and
	Y-prefixed terms.  The values must be of the format YYYYMMDDHHMM
        (e.g. 200702142359), YYYYMMDD (e.g. 20070214), or a raw 4 byte
        big-endian value representing a time_t (omindex adds this as value 0
	by default).

SPAN
	filter on this number of days up to END (if set), or after
	START (if set), or before today's date (otherwise).
	
START
	start of date range, in the format YYYYMMDD (defaults to 1st January
	1970).  If value-based date ranges are used (see DATEVALUE parameter)
	then the format YYYYMMDDHHMM is also valid.

END
	end of date range, in the format YYYYMMDD (defaults to today's date).
	If value-based date ranges are used (see DATEVALUE parameter) then the
	format YYYYMMDDHHMM is also valid.

xFILTERS
	used to spot when the filters have changed from the previous search.
	Set this to $html{$filters} in your query template ($filters is a
	compact serialisation of the currently set B filters, date-range
	filters, COLLAPSE, and DEFAULTOP).  If xFILTERS is unset, the filters
	are assumed not to have changed (unlike xP).  In Omega <= 1.2.21 and <=
	1.3.3 they were always assumed to have changed in the situation, which
	meant you couldn't ever go past page 1 if you failed to set xFILTERS
	in your template.  Now failing to set it means that the first page
	won't be forced some cases where it probably should be.

THRESHOLD
	apply a percentage cut-off at the value given by this parameter
	(clipped to the range 0-100).

Reordering parameters
---------------------

SORT
	reorder results by this value number.  The comparison used is a string
	compare of the unsigned byte values, and greater values are better
	by default (but this can be changed by setting SORTREVERSE to a
	non-zero value).

SORTREVERSE
	if non-zero, reverse the sort order so that lower values are better.
	This parameter has no effect unless SORT is also specified.

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
	ASCENDING order or any other character for DONT_CARE (the Xapian
	database backend will use whichever order is most efficient).  Any
	characters after the first are ignored.

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

For an image button, two CGI parameters are passed from the HTML
client, of the form "PARAM.x" and "PARAM.y" (the x and y coordinates
within the image that were clicked).

The PARAM part of the parameters are taken from the value attribute of
the <input> element that specified that image button in the HTML
page. We regularly use image buttons to provide pretty navigation
within search results (they are part of a form because it is easier to
treat more or less all of Omega as a single form, rather than
generating very long GET requests for every button on the results
page), so Omega does some mangling of these parameters:

 * PARAM.y is silently dropped
 * PARAM.x is truncated to PARAM
 * if PARAM contains a space (the CGI parameter name, not the value):
    * the value becomes everything after the first space; the
      original value is dropped. (e.g.: [ 2 ].x=NNN becomes [=2 ])

   otherwise:
    * if PARAM is entirely numeric, the name becomes '#' and the value
      becomes PARAM. (e.g.: 2.x=NNN becomes #=2)
    * if PARAM is not entirely numeric, the value is copied from PARAM
      (e.g.: >.x=NNN becomes >=>)

Then, for ALL CGI parameters, the name is truncated at the first
space. So [ page two ]=2 becomes [=2.
