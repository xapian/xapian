===================
Character Encodings
===================

The omega CGI assumes that text in the database is encoded as UTF-8.

When using omindex to index, this should automatically be the case - omindex
converts text extracted from documents to UTF-8 if it isn't already in this
encoding.  There's built-in code to handle the following: ISO-8859-1,
WINDOWS-1252, CP-1252, UTF-16, UCS-2, UTF-16BE, UCS-2BE, UTF-16LE, UCS-2LE.
And if built with iconv, many other encodings can be handled.

For plain text, omindex looks for a Byte Order Mark (BOM) to recognise
UTF-8, UTF-16BE, UTF-16LE, UTF-32BE and UTF-32LE.  Otherwise files are
assumed to be UTF-8, or ISO-8859-1 if they contain byte sequences which
aren't valid UTF-8.

When using scriptindex, you should ensure that text you feed to scriptindex is
UTF-8.
