===================
Character Encodings
===================

The omega CGI assumes that text in the database is encoded as UTF-8.

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
which make up multi-byte UTF-8 sequences will get encoded.  In Omega 1.3.3
and later, the OmegaScript `$prettyurl` command will reverse this encoding
for valid UTF-8 sequences, and so filenames should be shown with only the
bare minimum of characters escaped.

However, if your filenames aren't encoded in UTF-8, `$prettyurl` will leave
alone percent-encoded bytes for non-ASCII characters (it is possible it could
find a valid UTF-8 sequence in other data and so show the wrong character, but
this is unlikely in real-world data).  Everything should still work at least.

scriptindex
===========

When using scriptindex, you should ensure that text you feed to scriptindex is
UTF-8.
