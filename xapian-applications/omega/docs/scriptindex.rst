=============
Index scripts
=============

The basic format is one or more field names followed by a colon, followed by
one or more actions.  Some actions take an optional or required parameter.
The actions are applied in the specified order to each field listed, and
fields can be listed in several lines.

Here's an example::

 desc1 : unhtml index truncate=200 field=sample
 desc2 desc3 desc4 : unhtml index
 name : field=caption weight=3 index
 ref : field=ref boolean=Q unique=Q
 type : field=type boolean=XT

The actions are:

boolean[=PREFIX]
	index the text as a single boolean term (with prefix PREFIX).  If
	there's no text, no term is added.  Omega expects certain prefixes to
	be used for certain purposes - those starting "X" are reserved for user
	applications.  Q is reserved for a unique ID term.

date=FORMAT
	generate terms for date range searching.  If FORMAT is "unix", then the
	value is interpreted as a Unix time_t (seconds since 1970).  If
	FORMAT is "yyyymmdd", then the value is interpreted as an 8 digit
	string, e.g. 20021221 for 21st December 2002.  Unknown formats,
	and invalid values are ignored at present.

field[=FIELDNAME]
	add as a field to the Xapian record.  FIELDNAME defaults to the field
	name in the dumpfile.  It is valid to have more than one instance of
	a given field: all instances will be processed and stored in the
	Xapian record.

hash[=LENGTH]
	Xapian has a limit on the length of a term.  To handle arbitrarily
	long URLs as terms, omindex implements a scheme where the end of
	a long URL is hashed (short URLs are left as-is).  You can use this
	same scheme in scriptindex.  LENGTH defaults to 239, which if you
	index with prefix "U" produces url terms compatible with omindex.

index[=PREFIX]
	split text into words and index probabilistically (with prefix PREFIX
	if specified).

indexnopos[=PREFIX]
	split text into words and index probabilistically (with prefix PREFIX
	if specified), but don't include positional information in the
	database - this makes the database smaller, but phrase searching won't
	work.

load
	reads the contents of the file using the current text as the filename
	and then sets the current text to the contents.  If the file can't be
	loaded (not found, wrong permissions, etc) then a diagnostic message is
	sent to stderr and the current text is set to empty.  If the next
	action is truncate, then scriptindex is smart enough to know it only
	needs to load the start of a large file.

lower
	lowercase the text (useful for generating boolean terms)

spell
        Generate spelling correction data for any ``index`` or ``indexnopos``
        actions in the remainder of this list of actions.

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
	replaced (or deleted if the new record is otherwise empty).  You should
	also index the field as a boolean field using the same prefix so that
	the old record can be found.  In Omega, Q is reserved for use as the
	prefix of a unique term.

value=VALUESLOT
	add as a Xapian document value in slot VALUESLOT.  Values can be used
	for collapsing equivalent documents, sorting the MSet, etc.  If you
        want to perform numeric sorting, use the valuenumeric action instead.

valuenumeric=VALUESLOT
        Like value=VALUESLOT, this adds as a Xapian document value in slot
        VALUESLOT, but it encodes it for numeric sorting using
        Xapian::sortable_serialise().  Values set with this action can be
        used for numeric sorting of the MSet.

weight=FACTOR
	set the weighting factor to FACTOR (an integer) for any ``index`` or
        ``indexnopos`` actions in the remainder of this list of actions.  The
        default is 1.  Use this to add extra weight to titles, keyword fields,
        etc, so that words in them are regarded as more important by searches.

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
