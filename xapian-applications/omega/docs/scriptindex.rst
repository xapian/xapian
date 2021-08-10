=============
Index scripts
=============

The basic format is one or more field names followed by a colon, followed by
one or more actions.  Some actions take an optional or required parameter.

Since Omega 1.4.6, the parameter value can be enclosed in double quotes,
which is necessary if it contains whitespace; it's also needed for
parameter values containing a comma for actions which support multiple
parameters (such as ``split``) since there unquoted commas are interpreted
as separating parameters.

Since Omega 1.4.8, the following C-like escape sequences are supported
for parameter values enclosed in double quotes: ``\\``, ``\"``, ``\0``, ``\t``,
``\n``, ``\r``, and ``\x`` followed by two hex digits.

The actions are applied in the specified order to each field listed, and
a field can be listed in multiple lines.

Comments are allowed on a line by themselves, introduced by a ``#``.

Here's an example::

 desc1 : unhtml index truncate=200 field=sample
 desc2 desc3 desc4 : unhtml index
 # This is a comment.
 name : field=caption weight=3 index
 ref : boolean=Q unique=Q
 type : boolean=XT
 ref type : field

Don't put spaces around the ``=`` separating an action and its argument -
current versions allow spaces here (though this was never documented as
supported) but it leads to a missing argument quietly swallowing the next
action rather than using an empty value or giving an error, e.g. this takes
``hash`` as the field name, which is unlikely to be what was intended::

 url : field= hash boolean=Q unique=Q

Since 1.4.6 a deprecation warning is emitted for spaces before or after the
``=``.

The actions are:

boolean[=PREFIX]
	index the text as a single boolean term (with prefix PREFIX).  If
	there's no text, no term is added.  Omega expects certain prefixes to
	be used for certain purposes - those starting "X" are reserved for user
	applications.  ``Q`` is conventionally used as the prefix for a unique
	ID term.

date=FORMAT
        generate ``D``-, ``M``- and ``Y``-prefixed terms for date range
        searching (e.g. ``D20021221``, ``M200212`` and ``Y2002`` for the
        21st December 2002).  The following values for *FORMAT* are supported:

          * ``unix``: the value is interpreted as a Unix local time_t (seconds
            since the start of 1970 in the local timezone).
          * ``unixutc``: the value is interpreted as a Unix UTC time_t
            (seconds since the start of 1970 in UTC).  (Since Omega 1.4.12)
          * ``yyyymmdd``: the value is interpreted as an 8 digit string, e.g.
            20021221 for 21st December 2002.

        Unknown formats give an error at script parse time since Omega 1.4.12
        (in earlier versions unknown formats uselessly resulted in the terms
        ``D``, ``M`` and ``Y`` literally being added to every document).

        Invalid values result in no terms being added (and since Omega 1.4.12
        a warning is emitted).

field[=FIELDNAME]
	add as a field to the Xapian record.  FIELDNAME defaults to the field
	name in the dumpfile.  It is valid to have more than one instance of
	a given field: all instances will be processed and stored in the
	Xapian record.

gap[=SIZE]
        leave a gap of SIZE term positions.  SIZE defaults to 100.  This
        provides a way to stop phrases, ``NEAR`` and ``ADJ`` from matching
        across fields.

hash[=LENGTH]
	Xapian has a limit on the length of a term.  To handle arbitrarily
	long URLs as terms, omindex implements a scheme where the end of
	a long URL is hashed (short URLs are left as-is).  You can use this
	same scheme in scriptindex.  LENGTH defaults to 239, which if you
	index with prefix "U" produces url terms compatible with omindex.
        If specified, LENGTH must be at least 6 (because the hash takes 6
        characters).

hextobin
        converts pairs of hex digits to binary byte values (providing a way
        to specify arbitrary binary strings e.g. for use in a document value
        slot).  The input should have an even length and be composed entirely
        of hex digits (if it isn't, an error is reported and the value is
        unchanged).

        ``hextobin`` was added in Omega 1.4.6.

index[=PREFIX]
	split text into words and index (with prefix PREFIX if specified).

indexnopos[=PREFIX]
	split text into words and index (with prefix PREFIX if specified), but
	don't include positional information in the database - this makes the
	database smaller, but phrase searching won't work.

load
        reads the contents of the file using the current text as the filename
        and then sets the current text to the contents.  If the current text
        is empty, a warning is issued (since Xapian 1.4.10).  If the file can't
        be loaded (not found, wrong permissions, etc) then an error is issued and
        the current text is set to empty.

        If the next action is ``truncate``, then scriptindex is smart enough to
        know it only needs to load the start of a large file.

lower
	lowercase the text (useful for generating boolean terms)

ltrim[=CHARACTERSTOTRIM]
        remove leading characters from the text which are in
        ``CHARACTERSTOTRIM`` (default: space, tab, formfeed, vertical tab,
        carriage return, linefeed).

        Currently only ASCII characters are supported in ``CHARACTERSTOTRIM``.

        See also ``rtrim``, ``squash`` and ``trim``.

        ``ltrim`` was added in Omega 1.4.19.

parsedate=FORMAT
        parse the text as a date string using ``strptime()`` (or C++11's
        ``std::get_time()`` on platforms without ``strptime()``) with the
        format specified by ``FORMAT``, and set the text to the result as a
        Unix ``time_t`` (seconds since the start of 1970 in UTC), which can
        then be fed into ``date=unixutc`` or ``valuepacked``, for example::

         last_update : parsedate="%Y%m%d %T" field=lastmod valuepacked=0

        Format strings containing ``%Z`` are rejected with an error, as it
        seems that ``strptime()`` implementations don't properly support this
        (glibc's just accepts any sequence of non-whitespace and ignores it).

        Format strings containing ``%z`` are only supported on platforms
        where ``struct tm`` has a ``tm_gmtoff`` member, which is needed to
        correctly apply the timezone offset.  On other platforms ``%z`` is
        also rejected with an error.

        ``parsedate`` was added in Omega 1.4.6.

rtrim[=CHARACTERSTOTRIM]
        remove trailing characters from the text which are in
        ``CHARACTERSTOTRIM`` (default: space, tab, formfeed, vertical tab,
        carriage return, linefeed).

        Currently only ASCII characters are supported in ``CHARACTERSTOTRIM``.

        See also ``ltrim``, ``squash`` and ``trim``.

        ``rtrim`` was added in Omega 1.4.19.

spell
        Generate spelling correction data for any ``index`` or ``indexnopos``
        actions in the remainder of this list of actions.

split=DELIMITER[,OPERATION]
        Split the text at each occurrence of ``DELIMITER``, discard any empty
        strings, perform ``OPERATION`` on the resulting list, and then for each
        entry perform all the actions which follow ``split`` in the current rule.

        ``OPERATION`` can be ``dedup`` (remove second and subsequent
        occurrences from the list of any value), ``prefixes`` (which instead of
        just giving the text between delimiters, gives the text up to each
        delimiter), ``sort`` (sort), or ``none`` (default: none).

        If you want to specify ``,`` for delimiter, you need to quote it, e.g.
        ``split=",",dedup``.

squash[=CHARACTERSTOTRIM]
        replace runs of one or more characters from ``CHARACTERSTOTRIM`` in the
        text with a single space.  Leading and trailing runs are removed entirely.

        ``CHARACTERSTOTRIM`` defaults to: space, tab, formfeed, vertical tab,
        carriage return, linefeed).

        Currently only ASCII characters are supported in ``CHARACTERSTOTRIM``.

        See also ``ltrim``, ``rtrim`` and ``trim``.

        ``squash`` was added in Omega 1.4.19.

trim[=CHARACTERSTOTRIM]
        remove leading and trailing characters from the text which are in
        ``CHARACTERSTOTRIM`` (default: space, tab, formfeed, vertical tab,
        carriage return, linefeed).

        Currently only ASCII characters are supported in ``CHARACTERSTOTRIM``.

        See also ``ltrim``, ``rtrim`` and ``squash``.

        ``trim`` was added in Omega 1.4.19.

truncate=LENGTH
	truncate to at most LENGTH bytes, but avoid chopping off a word (useful
	for sample and title fields)

unhtml
	strip out HTML tags

unique[=PREFIX]
	use the value in this field for a unique ID.  If the value is empty,
	a warning is issued but nothing else is done.  Only one record with
	each value of the ID may be present in the index: adding a new record
	with an ID which is already present will cause the old record to be
        replaced or deleted.

        Deletion happens if the only input field present has the `unique`
        action applied to it.  (Prior to 1.5.0, if there were multiple lists
        of actions applied to an input field this triggered replacement instead
        of deletion).  If you want to suppress this deletion feature, supplying
        a dummy input field which doesn't match the index script will achieve
        this.

        You should also index the field as a boolean field using the same
        prefix so that the old record can be found.  In Omega, ``Q`` is
        conventionally used as the prefix of a unique term.

        You can use ``unique`` at most once in each index script (this is only
        enforced since Omega 1.4.5, but older versions didn't handle multiple
        instances usefully).

unxml
        strip out XML tags, replacing with a space (``unxml`` is similar to
        ``unhtml``, but ``unhtml`` varies the whitespace type or omits it
        entirely, based on HTML tag semantics).

        ``unxml`` was added in Omega 1.5.0.

value=VALUESLOT
	add as a Xapian document value in slot VALUESLOT.  Values can be used
	for collapsing equivalent documents, sorting the MSet, etc.  If you
        want to perform numeric sorting, use the valuenumeric action instead.

valuenumeric=VALUESLOT
        Like value=VALUESLOT, this adds as a Xapian document value in slot
        VALUESLOT, but it first encodes for numeric sorting using
        Xapian::sortable_serialise().  Values set with this action can be
        used for numeric sorting of the MSet.

valuepacked=VALUESLOT
        Like value=VALUESLOT, this adds as a Xapian document value in slot
        VALUESLOT, but it first encodes as a 4 byte big-endian binary string.
        If the input is a Unix time_t value, the resulting slot can be used for
        date range filtering and to sort the MSet by date.  Can be used in
        combination with ``parsedate``, for example::

         last_update : parsedate="%Y%m%d %T" field=lastmod valuepacked=0

        ``valuepacked`` was added in Omega 1.4.6.

weight=FACTOR
        set the weighting factor to FACTOR (a non-negative integer) for any
        ``index`` or ``indexnopos`` actions in the remainder of this list of
        actions.  The default is 1.  Use this to add extra weight to titles,
        keyword fields, etc, so that words in them are regarded as more
        important by searches.

Input files:
============

The data to be indexed is read in from one or more files.  Each file has
records separated by a blank line.  Each record contains one or more fields of
the form "name=value".  If value contains newlines, these must be escaped by
inserting an equals sign ('=') after each newline.  Here's an example record::

 id=ghq147
 title=Sample Record
 value=This is a multi-line
 =value.  Note how each newline
 =is escaped.
 format=HTML

Example:
========

See mbox2omega and mbox2omega.script for an example of how you can generate a
dump file from an external source and write an index script to be used with it.
Try "mbox2omega --help" for more information.
