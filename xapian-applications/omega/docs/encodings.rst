===================
Character Encodings
===================

The omega CGI assumes that text in the database is encoded as UTF-8.

If you are writing your own search form, it is best to ensure that the query
will be sent as UTF-8.  By default, web browsers will send the form parameters
with the same encoding as the page the form is on (and the default encoding for
HTML pages is ISO-8859-1).  You can override this by adding the parameter
`accept-charset="UTF-8"` to the `<form>` tag of your search form (and it's
safe to do this even in a page which is explicitly UTF-8).

If the form parameters get sent as ISO-8859-1, there are several issues:

The first is that characters which aren't representable in ISO-8859-1 get
sent as numeric HTML entities, such as `&#25991;`.  But there's no way
to distinguish these from the same text literally entered into the form
by the user.

The second is that Omega can't simply re-encode the form data as the
encoding used isn't specified in the form submission (whether that is by
GET or POST).

If Xapian is asked to parse a query string which isn't valid UTF-8, it will
fall-back to handling it as ISO-8859-1, which will usually do the right thing
for queries which are representable in ISO-8859-1.  However, things like
boolean filters in `B` parameters will be used as-is, so any which contain
non-ASCII characters won't work properly.

omindex
=======

When using omindex to index, this should automatically be the case - omindex
converts text extracted from documents to UTF-8 if it isn't already in this
encoding.  There's built-in code to handle the following: ISO-8859-1,
WINDOWS-1252, CP-1252, UTF-16, UCS-2, UTF-16BE, UCS-2BE, UTF-16LE, UCS-2LE.
And if built with iconv, many other encodings can be handled.

For plain text, omindex looks for a Byte Order Mark (BOM) to recognise
UTF-8, UTF-16BE, UTF-16LE, UTF-32BE and UTF-32LE.  Otherwise files are
assumed to be UTF-8, or ISO-8859-1 if they contain byte sequences which
aren't valid UTF-8.

When omindex builds URLs, it percent-encodes bytes according to RFC-3986.
On modern systems, filenames are usually encoded in UTF-8, and the bytes
which make up multi-byte UTF-8 sequences will get encoded.  In Omega 1.2.21
or 1.3.3 and later, the OmegaScript `$prettyurl` command will reverse this
encoding for valid UTF-8 sequences, and so filenames should be shown with only
the bare minimum of characters escaped.

However, if your filenames aren't encoded in UTF-8, `$prettyurl` will leave
alone percent-encoded bytes for non-ASCII characters (it is possible it could
find a valid UTF-8 sequence in other data and so show the wrong character, but
this is unlikely in real-world data).  Everything should still work at least.

scriptindex
===========

When using scriptindex, you should ensure that text you feed to scriptindex is
UTF-8.
