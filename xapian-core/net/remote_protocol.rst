Remote Backend Protocol
=======================

This document describes *version 45.0* of the protocol used by Xapian's
remote backend. The major protocol version increased to 45 in Xapian
1.5.0.

.. , and the minor protocol version to 1 in Xapian 1.2.4.

Clients and servers must support matching major protocol versions and the
client's minor protocol version must be the same or lower. This means that for
a minor protocol version change, you can upgrade first servers and then
clients and everything should work during the upgrades.

The protocol assumes a reliable two-way connection across which
arbitrary data can be sent - this could be provided by a TCP socket for
example (as it is with xapian-tcpsrv), but any such connection could be
used. For example, you could used xapian-progsrv across an ssh
connection, or even a custom server across a suitable serial connection.

All messages start with a single byte identifying code. A message from
client to server has a ``MSG_XXX`` identifying code, while a message
from server to client has a ``REPLY_XXX`` identifying code (but note
that a reply might not actually be in response to a message -
``REPLY_UPDATE`` is sent by the server when the connection is opened - and some
messages result in multiple replies).

The identifying code is followed by the encoded length of the contents
followed by the contents themselves.

Inside the contents, strings are generally passed as an encoded length
followed by the string data (this is indicated below by ``S<...>`` and
implemented by the ``pack_string()`` and ``unpack_string()`` functions)
except when the string is the last or only thing in the contents in
which case we know the length because we know the length of the contents
so we don't need to explicitly specify it (indicated by ``<...>`` below).

Unsigned integers are encoded using the same encoding used for string lengths
(indicated by ``I<...>`` below and implemented by the ``pack_uint()`` and
``unpack_uint()`` functions), except when the unsigned integer is the
last or only thing in the contents in which case bytes up to the most
significant non-zero byte are stored (indicated by ``L<...>`` and implemented
``pack_uint_last()`` and ``unpack_uint_last()`` functions).

Floating pointing values are passed in little-endian IEEE754 double format,
which means a fixed 8-byte size.  This is indicated by ``F<...>`` below.

Boolean values are passed as a single byte which is the ASCII character
value for ``0`` or ``1``. This is indicated by ``B<...>`` below, and
implemented by the ``pack_bool()`` and ``unpack_bool()`` functions).

Unsigned byte values are indicated by ``C<...>`` below.

Server statistics
-----------------

-  ``REPLY_UPDATE C<protocol major version> C<protocol minor version> I<db doc count> I<last_docid - db_doc_count> I<doclen_lower_bound> I<doclen_upper_bound - doclen_lower_bound> B<has positions?> I<db total length> <UUID>``

The protocol major and minor versions are passed as a single byte each
(e.g. ``'\x1e\x01'`` for version 30.1). The server and client must
understand the same protocol major version, and the server protocol
minor version must be greater than or equal to that of the client (this
means that the server understands newer MSG\_\ *XXX*, but will only send
newer REPLY\_\ *YYY* in response to an appropriate client message.

Exception
---------

-  ``REPLY_EXCEPTION <serialised Xapian::Error object>``

If an unknown exception is caught by the server, this message is sent
but with empty contents.

This message can be sent at any point - the serialised exception is
unserialised by the client and thrown. The server and client both abort
any current sequence of messages.

Write Access
------------

-  ``MSG_WRITEACCESS``
-  ``REPLY_UPDATE [...]``

The reply message is the same format as the server's opening greeting given
above.

If write access isn't supported or the database is locked by another writer,
then an exception is thrown.

By default the server is also read-only, even if writing is supported.
If the client wants to be able to write, it needs to request this
explicitly. We do this so that the same server can support multiple
read-only clients and one writing client at once, without the protocol
for read-only clients requiring an extra message. The overhead of an
extra message exchange for a writer is unlikely to matter as indexing is
rarely so real-time critical as searching.

All Terms
---------

-  ``MSG_ALLTERMS <prefix>``
-  ``REPLY_ALLTERMS [C<chars of previous term to reuse> S<string to append> I<term freq>]...``

Term Exists
-----------

-  ``MSG_TERMEXISTS <term name>``
-  ``REPLY_TERMEXISTS`` or ``REPLY_TERMDOESNTEXIST``

Term Frequency
--------------

-  ``MSG_TERMFREQ <term name>``
-  ``REPLY_TERMFREQ L<term freq>``

Collection Frequency
--------------------

-  ``MSG_COLLFREQ <term name>``
-  ``REPLY_COLLFREQ L<collection freq>``

Freqs
-----

-  ``MSG_FREQS <term name>``
-  ``REPLY_FREQS I<term freq> L<collection freq>``

Unique Terms
------------

-  ``MSG_UNIQUETERMS L<document id>``
-  ``REPLY_UNIQUETERMS L<number of unique terms>``

Max wdf
-------

-  ``MSG_WDFDOCMAX L<document id>``
-  ``REPLY_WDFDOCMAX L<max wdf in the document>``

Value Stats
-----------

-  ``MSG_VALUESTATS L<value no>``
-  ``REPLY_VALUESTATS I<freq> S<lower bound> <upper bound>``

Document
--------

-  ``MSG_DOCUMENT L<document id>``
-  ``REPLY_DOCDATA <document data>``
-  ``REPLY_VALUE I<value no> <value>``
-  ``...``
-  ``REPLY_DONE``

Document Length
---------------

-  ``MSG_DOCLENGTH L<document id>``
-  ``REPLY_DOCLENGTH L<document length>``

Keep Alive
----------

-  ``MSG_KEEPALIVE``
-  ``REPLY_DONE``

Reopen
------

-  ``MSG_REOPEN``
-  ``REPLY_DONE`` or ``REPLY_UPDATE [...]``

If the database was already at the latest version, ``REPLY_DONE`` is returned.

If it was reopened, then the reply message is the same format as the server's
opening greeting given above.

Query
-----

-  ``MSG_QUERY S<serialised Xapian::Query object> I<query length> I<collapse max> [I<collapse key number> (if collapse_max non-zero)] C<docid order> C<sort by> [I<sort key number> (if sort_by non-zero)] B<sort value forward> B<full db has positions> F<time limit> C<percent threshold> F<weight threshold> S<Xapian::Weight class name> S<serialised Xapian::Weight object> S<serialised Xapian::RSet object> [S<Xapian::MatchSpy class name> S<serialised Xapian::MatchSpy object>]...``
-  ``REPLY_STATS <serialised Stats object>``
-  ``MSG_GETMSET I<first> I<max items> I<check at least> S<sorter name> [L<serialised Xapian::Sorter object>] <serialised global Stats object>``
-  ``REPLY_RESULTS [S<result of calling serialise_results() on Xapian::MatchSpy>]... <serialised Xapian::MSet object>``

docid order is ``0``, ``1`` or ``2``.

sort by is ``0``, ``1``, ``2`` or ``3``.

If there's no sorter then ``<sorter name>`` is empty and
``L<serialised Xapian::Sorter object>`` is omitted.

Termlist
--------

-  ``MSG_TERMLIST L<document id>``
-  ``REPLY_TERMLISTHEADER I<document length> L<number of entries>``
-  ``REPLY_TERMLIST [C<chars of previous term to reuse> S<string to append> I<wdf> I<term freq> ]...``

Positionlist
------------

-  ``MSG_POSITIONLIST I<document id> <term name>``
-  ``REPLY_POSITIONLIST [I<termpos delta - 1>]...``

Since positions must be strictly monotonically increasing, we encode
``(pos - lastpos - 1)`` so that small differences between large position
values can still be encoded compactly. The first position is encoded as
its true value.

Positionlist count
------------------

-  ``MSG_POSITIONLISTCOUNT I<document id> <term name>``
-  ``REPLY_POSITIONLISTCOUNT L<count>``

Get the length of the positionlist without fetching the list itself.

Postlist
--------

-  ``MSG_POSTLIST <term name>``
-  ``REPLY_POSTLISTHEADER L<termfreq>``
-  ``REPLY_POSTLIST [I<docid delta - 1> I<wdf>]...``

Since document IDs in postlists must be strictly monotonically
increasing, we encode ``(docid - lastdocid - 1)`` so that small
differences between large document IDs can still be encoded compactly.
The first document ID is encoded as its true value - 1 (since document
IDs are always > 0).

Shut Down
---------

-  ``MSG_SHUTDOWN``

No reply is sent - this message signals that the client has ended the
session.

Update
------

-  ``MSG_UPDATE``
-  ``REPLY_UPDATE [...]``

Only useful for a ``WritableDatabase`` (since the same statistics are
sent when the connection is initiated in the ``REPLY_GREETING`` and they
don't change if the database can't change).

Add document
------------

-  ``MSG_ADDDOCUMENT <serialised Xapian::Document object>``
-  ``REPLY_ADDDOCUMENT L<document id>``

Delete document
---------------

-  ``MSG_DELETEDOCUMENT L<document id>``
-  ``REPLY_DONE``

Delete document by term
-----------------------

-  ``MSG_DELETEDOCUMENTTERM <term name>``
-  ``REPLY_DONE``

Replace document
----------------

-  ``MSG_REPLACEDOCUMENT I<document id> <serialised Xapian::Document object>``
-  ``REPLY_DONE``

Replace document by term
------------------------

-  ``MSG_REPLACEDOCUMENTTERM S<term name> <serialised Xapian::Document object>``
-  ``REPLY_ADDDOCUMENT I<document id>``

Cancel
------

-  ``MSG_CANCEL``
-  ``REPLY_DONE``

Commit
------

-  ``MSG_COMMIT``
-  ``REPLY_DONE``

Set metadata
------------

-  ``MSG_SETMETADATA S<key> <value>``
-  ``REPLY_DONE``

Get metadata
------------

-  ``MSG_GETMETADATA <key>``
-  ``REPLY_METADATA <value>``

Metadata keys
-------------

-  ``MSG_METADATAKEYLIST <prefix>``
-  ``REPLY_METADATAKEYLIST [C<chars of previous term to reuse> S<string to append>]...``

Add spelling
------------

-  ``MSG_ADDSPELLING I<freqinc> <word>``
-  ``REPLY_DONE``

Remove spelling
---------------

-  ``MSG_REMOVESPELLING I<freqdec> <word>``
-  ``REPLY_REMOVESPELLING L<result>``

Reconstruct text
----------------

-  ``MSG_RECONSTRUCTTEXT I<did> I<length> I<start_pos> I<end_pos> <prefix>``
-  ``REPLY_RECONSTRUCTTEXT <text>``

Synonym Term List
-----------------

-  ``MSG_SYNONYMTERMLIST <word>``
-  ``REPLY_SYNONYMTERMLIST [C<chars of previous term to reuse> S<string to append>]...``

Synonym Key List
----------------

-  ``MSG_SYNONYMKEYLIST <word>``
-  ``REPLY_SYNONYMKEYLIST [C<chars of previous term to reuse> S<string to append>]...``

Add synonym
-----------

- ``MSG_ADDSYNONYM S<word> <synonym>
- ``REPLY_DONE``

Remove synonym
--------------

- ``MSG_REMOVESYNONYM S<word> <synonym>
- ``REPLY_DONE``

Clear synonyms
--------------

- ``MSG_CLEARSYNONYMS <word>``
- ``REPLY_DONE``
