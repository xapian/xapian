===========
OmegaScript
===========

OmegaScript adds processed text-generation commands to text templates
(which will usually be HTML, but can be XML or another textual format).
Commands take the form ``$command{comma,separated,arguments}`` or
``$simplecommand``, for example::

    <html>
    <head><title>Sample</title></head>
    <body>

    <p>
    You searched for '$html{$query}'.
    </p>

    </body>
    </html>

Where appropriate, arguments themselves can contain OmegaScript commands.
Where an argument is treated as a string, the string is precisely the contents
of that argument - there is no string delimiter (such as the double-quote
character '"' in C and similar languages).  This can make complex OmegaScript
slightly difficult to read at times.

When a command takes no arguments, the braces must be omitted (i.e.
`$query` rather than `$query{}` - the latter is a command with a
single empty argument).  If you want to have the value of `$query` immediately
followed by a letter, digit, or "_", you can use an empty comment (`${}`) to
prevent the parser treating the following character as part of a command name.
E.g. `_$query${}_` rather than `$query_`

It is important to realise that all whitespace is significant in OmegaScript
- e.g. if you put whitespace around a "," which separates two command arguments
then the whitespace will be part of the respective arguments.

Note that (by design) OmegaScript has no unbounded looping constructs.  You
can loop over entries in a list, but you can't loop until some arbitrary
condition is met.  This means that it's not possible to accidentally (or
deliberately!) write an OmegaScript template which contains an infinite loop.

OmegaScript literals
====================

::

    $$ - literal '$'
    $( - literal '{'
    $) - literal '}'
    $. - literal ','


OmegaScript commands
====================

In the following descriptions, a LIST is a string of tab-separated
values.

${...}
	commented-out code

$addfilter{TERM}
        add filter term ``TERM`` as if it had been passed as a ``B`` CGI
        parameter.  You must use ``$addfilter`` before any command which
        requires the query to have been parsed - see ``$setmap`` for a list
        of these commands.

$allterms{docid}
	list of all terms matching document

$cgi{CGI}
        lookup the value of a CGI parameter.  If the same parameter has
        multiple values, ``$cgi`` will pick one arbitrarily - use ``$cgilist``
        if you want all the values.

$cgilist{CGI}
	return a list of all values of a CGI parameter

$collapsed
        number of other documents collapsed into current hit inside
        ``$hitlist``, which might be used like so::

             $if{$ne{$collapsed,0},at least $collapsed hidden results ($value{$cgi{COLLAPSE}})}

$date{TIME_T[,FMT]}
	convert a time_t to strftime ``FMT`` (default: ``YYYY-MM-DD``).  The
	conversion is done in timezone UTC.

$dbname
	database name (multiple names are returned separated by "/").

$dbsize
	number of documents in the database (if multiple databases are being
	searched, this gives the total number).

$def{MACRONAME,VALUE}
	define a macro which can take 0 to 9 arguments.  You can call it with
        ``$MACRONAME`` (if it take 0 arguments) or
        ``$MACRONAME{ARG1,ARG2,ARG3}`` is it takes arguments.  In value,
        arguments are available as ``$1``, ``$2``, ...  ``$9``.  In the current
        implementation, macros can override OmegaScript commands, but this
        shouldn't be relied on.  It's recommended to use capitalised names for
        macros to avoid collision with future OmegaScript commands.

$defaultop
	"and" or "or" (set from CGI variable DEFAULTOP).

$emptydocs[{TERM}]
	returns a list of docids of any documents with document length zero
	(such documents probably only contain scanned images, rather than
	machine readable text, or suggest the input filter isn't working well).
	If TERM is specified, only consider documents matching TERM, otherwise
	all documents are considered (so Tapplication/pdf reports all PDF files
	for which no text was found).

	If you're using omindex, note that it skips files with zero size, so
	these won't get reported here as they aren't present in the database.

$env{VAR}
	lookup variable ``VAR`` in the environment.

$error
	error message (e.g. if a database wouldn't open, or the query couldn't
        be parsed, or a Xapian exception has been thrown) or empty if there
	wasn't an error.

$field{NAME[,DOCID]}
	lookup field ``NAME`` in document ``DOCID``.  If ``DOCID`` is omitted
	then the current hit is used (which only works inside ``$hitlist``).

	If multiple instances of field exist the field values are returned tab
	separated, which means you can pass the results to ``$map``, e.g.::

            $map{$field{keywords},<b>$html{$_}</b><br>}

$filesize{SIZE}
	pretty printed filesize (e.g. ``1 byte``, ``100 bytes``, ``2.1K``,
        ``4.0M``, ``1.3G``).  If ``SIZE`` is negative, expands to nothing.

$filters
        serialised version of filter-like settings (currently ``B``, ``START``,
        ``END``, ``SPAN``, ``COLLAPSE``, ``DOCIDORDER``, ``SORT``,
        ``SORTREVERSE``, ``SORTAFTER``, and ``DEFAULTOP``) - set ``xFILTERS``
        to this so that Omega can detect when the filters have changed and
        force the first page.

$filterterms{PREFIX}
        list of all terms in the database with prefix ``PREFIX``, intended to
        be used to allow drop-down lists and sets of radio buttons to be
	dynamically generated, e.g.::

             Hostname:
             <SELECT NAME="B">
             <OPTION VALUE=""
             $if{$map{$cgilist{B},$eq{$substr{$_,0,1},H}},,SELECTED}> Any
             $map{$filterterms{H},
             <OPTION VALUE="$html{$_}" $if{$find{$cgilist{B},$html{$_}},SELECTED}>
             $html{$substr{$_,1}}
             </OPTION>
             }
             </SELECT>

$find{LIST,STRING}
        returns the number of the first entry in ``LIST`` which is equal to
        ``STRING`` (starting from 0) or the empty string if no entry matches.

$fmt
	name of current format (as set by CGI parameter``FMT``, or the default)

$freq{term}
	frequency of a term

$highlight{TEXT,LIST,[OPEN,[CLOSE]]}
	html escape string (<>&, etc) and highlight any terms from ``LIST``
        that appear in ``TEXT`` by enclosing them in ``OPEN`` and ``CLOSE``.
        If ``OPEN`` is specified, but close is omitted, ``CLOSE`` defaults to
        the appropriate closing tag for ``OPEN`` (i.e. with a "/" in front and
        any parameters removed).  If both are omitted, then ``OPEN`` is set to:
	``<b style="color:XXXXX;background-color:#YYYYYY">`` (where ``YYYYYY``
        cycles through ``ffff66`` ``99ff99`` ``99ffff`` ``ff66ff`` ``ff9999``
        ``990000`` ``009900`` ``996600`` ``006699`` ``990099`` and ``XXXXX``
        is ``black`` is ``YYYYYY`` contains an ``f``, and otherwise ``white``)
        and ``CLOSE`` is ``</b>``.

$hit
	MSet index of current doc (first document in MSet is 0, so if
	you want to number the hits 1, 2, 3, ... use ``$add{$hit,1}``).

$hitlist{FMT}
	display hitlist using format ``FMT``.

$hitsperpage
	hits per page (as set by ``HITSPERPAGE``, or the default)

$hostname{URL}
	return the hostname from url ``URL``

$html{TEXT}
	html escape string (``<>&"`` are escaped to ``&lt;``, etc).

$htmlstrip{TEXT}
	html strip tags from string (``<...>``, etc).

$httpheader{NAME,VALUE}
	specify an additional HTTP header to be generated by Omega.
	For example::

	 $httpheader{Cache-Control,max-age=0$.private}

	If ``Content-Type`` is not specified by the template, it defaults
	to ``text/html``.  Headers must be specified before any other
	output from the OmegaScript template - any ``$httpheader{}``
	commands found later in the template will be silently ignored.

$id
	document id of current document

$last
        MSet index one beyond the end of the current page (so ``$hit`` runs
        from ``0`` to ``$sub{$last,1}``).

$lastpage
	number of last page of hits (may be an underestimate unless
	``$thispage`` == ``$lastpage``).

$length{LIST}
	number of entries in ``LIST``.

$list{LIST,...}
	pretty print list. If ``LIST`` contains 1, 2, 3, 4 then::

	 "$list{LIST,$. }" = "1, 2, 3, 4"
	 "$list{LIST,$. , and }" = "1, 2, 3 and 4"
	 "$list{LIST,List ,$. ,.}" = "List 1, 2, 3, 4."
	 "$list{LIST,List ,$. , and ,.}" = "List 1, 2, 3 and 4."

	NB ``$list`` returns an empty string for an empty list (so the
	last two forms aren't redundant as it may at first appear).

$log{LOGFILE[,ENTRY]}
        write to the log file ``LOGFILE`` in directory ``log_dir`` (set in
        ``omega.conf``).  ``ENTRY`` is the OmegaScript for the log entry, and a
        linefeed is appended.  If ``LOGFILE`` cannot be opened for writing,
        nothing is done (and ``ENTRY`` isn't evaluated).  ``ENTRY`` defaults to
        a format similar to the Common Log Format used by webservers.

$lookup{CDBFILE,KEY}
        Return the tag corresponding to key ``KEY`` in the CDB file
        ``CDBFILE``.  If the file doesn't exist, or ``KEY`` isn't a key in it,
        then ``$lookup`` expands to nothing.  CDB files are compact disk based
        hashtables.  For more information and public domain software which can
        create CDB files, please visit: http://www.corpit.ru/mjt/tinycdb.html

	An example of how this might be used is to map top-level domains to
	country names.  Create a CDB file tld_en which maps "fr" to "France",
	"de" to "Germany", etc and then you can translate a country code to
	the English country name like so::

	 "$or{$lookup{tld_en,$field{tld}},.$field{tld}}"

	If a tld isn't in the CDB (e.g. "com"), this will expand to ".com".

	You can take this further and prepare a set of CDBs mapping tld codes
	to names in other languages - tld_fr for French, tld_de for German.
        Then if you have the ISO language code in ``$opt{lang}`` you can
        replace ``tld_en`` with ``tld_$or{$opt{lang},en}`` and automatically
        translate into the currently set language, or English if no language is
        set.

$lower{TEXT}
	return UTF-8 text ``TEXT`` converted to lower case.

$map{LIST,STUFF)
	map a list into the evaluated argument. If ``LIST`` is
	1, 2 then::

	 "$map{LIST,x$_ = $_; }" = "x1 = 1;	x2 = 2; "

	Note that $map{} returns a list (this is a change from older
	versions). If the tabs are a problem, use $list{$map{...},}
	to get rid of them.

$msize
	estimated number of matches.

$msizeexact
	return ``true`` if ``$msize`` is exact (or "" if it is estimated).

$nice{number}
	pretty print integer (with thousands separator).

$now
	number of seconds since the epoch (suitable for feeding to ``$date``).
	Whether ``$now`` returns the same value for repeated calls in the same
	Omega search session is unspecified.

$opt{OPT}
	lookup an option value (as set by ``$set``).

$opt{MAP,OPT}
	lookup an option within a map (as set by ``$setmap``).

$pack{NUMBER}
	converts a number to a 4 byte big-endian binary string

$percentage
	percentage score of current hit (in range 1-100).

	You probably don't want to show these percentage scores to end
	users in new applications - they're not really a percentage of
	anything meaningful, and research seems to suggest that users
	don't find numeric scores in search results useful.

$prettyterm{TERM}
	convert a term to "user form", as it might be entered in a query.  If
	a matching term was entered in the query, just use that (the first
	occurrence if a term was generated multiple times from a query).
	Otherwise term prefixes are converted back to user forms as specified
	by ``$setmap{prefix,...}`` and ``$setmap{boolprefix,...}``.

$prettyurl{URL}
	Prettify URL.  This command undoes RFC3986 URL escaping which doesn't
	affect semantics in practice, in order to make a prettier version of a
	URL for displaying to the user (rather than in links), but which should
	still work if copied and pasted.

$query
	query string (built from CGI ``P`` variable(s) plus possible added
	terms from ``ADD`` and ``X``).

$querydescription
        a human readable description of the ``Xapian::Query`` object which
        omega builds.  Mostly useful for debugging omega itself.

$queryterms
	list of probabilistic query terms.

$range{START,END}
	return list of values between ``START`` and ``END``.

$record[{ID}]
	raw record contents of document ``ID``.

$relevant[{ID}]
	document id ``ID`` if document is relevant, "" otherwise
	(side-effect: removes id from list of relevant documents
	returned by ``$relevants``).

$relevants
	return list of relevant documents

$score
	score (0-10) of current hit (equivalent to ``$div{$percentage,10}``).

$set{OPT,VALUE}
	set option value which may be looked up using ``$opt``.  You can use
	options as variables (for example, to store values you want to reuse
	without recomputing).  There are also several which Omega looks at
	and which you can set or use:

	* decimal - the decimal separator ("." by default - localised query
	  templates may want to set this to ",").
	* thousand - the thousands separator ("," by default - localised query
	  templates may want to set this to ".", " ", or "").
	* stemmer - which stemming language to use ("english" by default, other
	  values are as understood by ``Xapian::Stem``, so "none" means no
	  stemming).
	* stem_all - if "true", then tell the query parser to stem all words,
	  even capitalised ones.
	* spelling - if "true", then the query parser spelling correction
	  feature is enabled and ``$suggestion`` can be used.  Deprecated -
	  use flag_spelling_correction instead (which was added in version
	  1.2.5).
	* fieldnames - if set to a non-empty value then the document data is
	  parsed with each line being the value of a field, and the names
	  are taken from entries in the list in fieldnames.  So
          ``$set{fieldnames,$split{title sample url}}`` will take the first
          line as the "title" field, the second as the "sample" field and the
	  third as the "url" field.  Any lines without a corresponding field
	  name will be ignored.  If unset or empty then the document data is
	  parsed as one field per line in the format NAME=VALUE (where NAME is
	  assumed not to contain '=').
        * weighting - set the weighting scheme to use, and (optionally) the
          parameters to use if the weighting scheme supports them.  The syntax
          is a string consisting of the scheme name followed by any parameters,
          all separated by whitespace.  Any parameters not specified will use
          their default values.  Valid scheme names are ``bm25``, ``bool``, and
          ``trad``.  e.g. ``$set{weighting,bm25 1 0.8}``

        * expansion - (supported in 1.2.18 and later) set the query expansion
          scheme to use, and (optionally) the parameters to use if the
          expansion scheme supports them. The syntax is a string consisting of
          the scheme name followed by any parameters, all separated by
          whitespace.  Any parameters not specified will use their default
          values.  The only valid expansion scheme name in 1.2.x is ``trad``.
          e.g. ``$set{expansion,trad 2.0}``

	Omega 1.2.5 and later support the following options, which can be set
	to a non-empty value to enable the corresponding ``QueryParser`` flag.
	Omega sets ``flag_default`` to ``true`` by default - you can set it to
	an empty value to turn it off (``$set{flag_default,}``):

	* flag_auto_multiword_synonyms
	* flag_auto_synonyms
	* flag_boolean
	* flag_boolean_any_case
	* flag_cjk_ngram (new in 1.2.22 and 1.3.4)
	* flag_default
	* flag_lovehate
	* flag_partial
	* flag_phrase
	* flag_pure_not
	* flag_spelling_correction
	* flag_synonym
	* flag_wildcard

$setrelevant{docids}
	add documents into the RSet

$setmap{MAP,NAME1,VALUE1,...}
	set a map of option values which may be looked up against using
	``$opt{MAP,NAME}`` (maps with the same name are merged rather than
	the old map being completely replaced).

	You can create and use of maps in your own templates, but Omega also
	has several standard maps used to control building the query:

	Omega uses the "prefix" map to set the prefixes understood by the query
	parser.  So if you wish to translate a prefix of "author:" to A and
	"title:" to "S" you would use::

	 $setmap{prefix,author,A,title,S}

	Similarly, if you want to be able to restrict a search with a
	boolean filter from the text query (e.g. "group:" to "G") you
	would use::

	 $setmap{boolprefix,group,G}

	Don't be tempted to add whitespace around the commas, unless you want
	it to be included in the names and values!

	Note: you must set the prefix maps before the query is parsed.  This
	is done as late as possible - the following commands require the
	query to be parsed: $prettyterm, $query, $querydescription, $queryterms,
	$relevant, $relevants, $setrelevant, $unstem, and also these commands
	require the match to be run which requires the query to be parsed:
	$freqs, $hitlist, $last, $lastpage, $msize, $msizeexact, $terms,
	$thispage, $time, $topdoc, $topterms.

$slice{LIST,POSITIONS}
	returns the elements from ``LIST`` at the positions listed in the
	second list ``POSITIONS``.  The first item is at position 0.
	Any positions which are out of range will be ignored.

	For example, if ``LIST`` contains a, b, c, d then::

	 "$slice{LIST,2}" = "c"
	 "$slice{LIST,1	3}" = "b	d"
	 "$slice{LIST,$range{1,3}}" = "b	c	d"
	 "$slice{LIST,$range{-10,10}}" = "a	b	c	d"

$split{STRING}

$split{SPLIT,STRING}
	returns a list by splitting the string ``STRING`` into elements at each
        occurrence of the substring ``SPLIT``.  If ``SPLIT`` isn't specified,
        it defaults to a single space.  If ``SPLIT`` is empty, ``STRING`` is
        split into individual bytes.

	For example::

	 "$split{one two three}" = "one	two	three"

$stoplist
	returns a list of any terms in the query which were ignored as
	stopwords.

$substr{STRING,START[,LENGTH]}
        returns the substring of ``STRING`` which starts at byte position
        ``START`` (the start of the string being 0) and is ``LENGTH`` bytes
        long (or to the end of ``STRING`` if ``STRING`` is less than
        ``START``+``LENGTH`` bytes long).  If ``LENGTH`` is omitted, the
        substring from ``START`` to the end of ``STRING`` is returned.

	If ``START`` is negative, it counts back from the end of ``STRING`` (so
	``$substr{hello,-1}`` is ``o``).

	If LENGTH is negative, it instead specifies the number of bytes
	to omit from the end of STRING (so "$substr{example,2,-2}" is "amp").
	Note that this means that "$substr{STRING,0,N}$substr{STRING,N}" is
	"STRING" whether N is positive, negative or zero.

$suggestion
	if ``$set{spelling,true}`` was done before the query was parsed, then
	``$suggestion`` will return any suggested spelling corrected version
	of the query string.  If there are no spelling corrections, it will
	return an empty string.

$terms
	list of matching terms for current hit.

$thispage
	page number of current page.

$time
	how long the match took (in seconds) e.g. ``0.078534``.  If no timing
	information was available, returns an empty value.

$topdoc
	first document on current page of hit list (counting from 0)

$topterms[{N}]
	list of up to ``N`` top relevance feedback terms (default 16)

$transform{REGEXP,SUBST,STRING}
	transform string using Perl-compatible regular expressions.  This
	command is sort of like the Perl code::

         my $string = STRING;
         $string =~ s/REGEXP/SUBST/;
         print $string;

        In SUBST, ``\1`` to ``\9`` are substituted by the 1st to 9th bracket
        grouping (or are empty if there is no such bracket grouping).  ``\\``
        is a literal backslash.

$uniq{LIST}
	remove duplicates from a sorted list

$unpack{BINARYSTRING}
	converts a 4 byte big-endian binary string to a number, for example::

         $date{$unpack{$value{0}}}

$unstem{TERM}
	maps a stemmed term to a list of the unstemmed forms of it used in
	the query

$upper{TEXT}
	return UTF-8 text ``TEXT`` converted to upper case.

$url{TEXT}
	url encode argument

$value{VALUENO[,DOCID]}
        returns value number ``VALUENO`` for document ``DOCID``.  If ``DOCID``
        is omitted then the current hit is used (which only works inside
        ``$hitlist``).

$version
	omega version string - e.g. "xapian-omega 1.2.6"

$weight
	raw document weight of the current hit, as a floating point value
	(mostly useful for debugging purposes).

Numeric Operators:
==================

$add{...}
	add arguments together (if called with one argument, this will convert
	it to a string and back, which ensures it is an integer).

$div{A,B}
	returns int(A / B) (or the text "divide by 0" if B is zero)

$mod{A,B}
	returns int(A % B) (or the text "divide by 0" if B is zero)

$max{A,...}
	maximum of the arguments

$min{A,...}
	minimum of the arguments

$mul{A,B,...}
multiply arguments together

$muldiv{A,B,C}
	returns int((A * B) / C) (or the text "divide by 0" if C is zero)

$sub{A,B}
	returns (A - B)

Logical Operators:
==================

$and{...}
	logical short-cutting "and" of its arguments - evaluates
	arguments until it finds an empty one (and returns "") or
	has evaluated them all (returns "true")

$eq{A,B}
	returns "true" if A and B are the same, "" otherwise.

$ge{A,B}
	returns "true" if A is numerically >= B.

$gt{A,B}
	returns "true" if A is numerically > B.

$le{A,B}
	returns "true" if A is numerically <= B.

$lt{A,B}
	returns "true" if A is numerically < B.

$ne{A,B}
	returns "true" if A and B are not the same, "" if they are.

$not{A}
	returns "true" for the empty string, "" otherwise.

$or{...}
	logical short-cutting "or" of its arguments - returns first
	non-empty argument

Control:
========

$if{COND,THEN[,ELSE]}
	if ``COND`` is non-empty, evaluate ``THEN``, otherwise evaluate else
	(if present)

$include{FILE}
	include another OmegaScript file
