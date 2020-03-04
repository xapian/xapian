Remote Backend Protocol
=======================

This document describes *version 39.1* of the protocol used by Xapian's
remote backend. The major protocol version increased to 39 in Xapian
1.3.3, and the minor protocol version to 1 in Xapian 1.4.12.

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
followed by the string data (this is indicated below by ``L<...>``)
except when the string is the last or only thing in the contents in
which case we know the length because we know the length of the contents
so we don't need to explicitly specify it (indicated by ``<...>`` below).

Integers are encoded using the same encoding used for string lengths
(indicated by ``I<...>`` below).

Floating pointing values are passed using a bit packed encoding of the
sign and exponent and a base-256 encoding of the mantissa which avoids
any rounding issues (assuming that both machines have ``FLT_RADIX`` equal
to some power of 2). This is indicated by ``F<...>`` below.

Boolean values are passed as a single byte which is the ASCII character
value for ``0`` or ``1``. This is indicated by ``B<...>`` below.

Unsigned byte values are indicated by ``C<...>`` below.

Server statistics
-----------------

-  ``REPLY_UPDATE <protocol major version> <protocol minor version> I<db doc count> I(<last docid> - <db doc count>) I<doclen lower bound> I(<doclen upper bound> - <doclen lower bound>) B<has positions?> I<db total length> <UUID>``

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

-  ``MSG_ALLTERMS``
-  ``REPLY_ALLTERMS I<term freq> C<chars of previous term to reuse> <string to append>``
-  ``...``
-  ``REPLY_DONE``

Term Exists
-----------

-  ``MSG_TERMEXISTS <term name>``
-  ``REPLY_TERMEXISTS`` or ``REPLY_TERMDOESNTEXIST``

Term Frequency
--------------

-  ``MSG_TERMFREQ <term name>``
-  ``REPLY_TERMFREQ I<term freq>``

Collection Frequency
--------------------

-  ``MSG_COLLFREQ <term name>``
-  ``REPLY_COLLFREQ I<collection freq>``

Document
--------

-  ``MSG_DOCUMENT I<document id>``
-  ``REPLY_DOCDATA <document data>``
-  ``REPLY_VALUE I<value no> <value>``
-  ``...``
-  ``REPLY_DONE``

Document Length
---------------

-  ``MSG_DOCLENGTH I<document id>``
-  ``REPLY_DOCLENGTH I<document length>``

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

-  ``MSG_QUERY L<serialised Xapian::Query object> I<query length> I<collapse max> [I<collapse key number> (if collapse_max non-zero)] <docid order> I<sort key number> <sort by> B<sort value forward> F<time limit> <percent cutoff> F<weight cutoff> <serialised Xapian::Weight object> <serialised Xapian::RSet object> [L<serialised Xapian::MatchSpy object>...]``
-  ``REPLY_STATS <serialised Stats object>``
-  ``MSG_GETMSET I<first> I<max items> I<check at least> <serialised global Stats object>``
-  ``REPLY_RESULTS L<the result of calling serialise_results() on each Xapian::MatchSpy> <serialised Xapian::MSet object>``

docid order is ``'0'``, ``'1'`` or ``'2'``.

sort by is ``'0'``, ``'1'``, ``'2'`` or ``'3'``.

Termlist
--------

-  ``MSG_TERMLIST I<document id>``
-  ``REPLY_DOCLENGTH I<document length>``
-  ``REPLY_TERMLIST I<wdf> I<term freq> C<chars of previous term to reuse> <string to append>``
-  ``...``
-  ``REPLY_DONE``

Positionlist
------------

-  ``MSG_POSITIONLIST I<document id> <term name>``
-  ``REPLY_POSITIONLIST I<termpos delta - 1>``
-  ``...``
-  ``REPLY_DONE``

Since positions must be strictly monotonically increasing, we encode
``(pos - lastpos - 1)`` so that small differences between large position
values can still be encoded compactly. The first position is encoded as
its true value.

Postlist
--------

-  ``MSG_POSTLIST <term name>``
-  ``REPLY_POSTLISTSTART I<termfreq> I<collfreq>``
-  ``REPLY_POSTLISTITEM I<docid delta - 1> I<wdf> F<document length>``
-  ``...``
-  ``REPLY_DONE``

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
-  ``REPLY_UPDATE I<db doc count> I<last docid> B<has positions?> I<db total length> <UUID>``

Only useful for a ``WritableDatabase`` (since the same statistics are
sent when the connection is initiated in the ``REPLY_GREETING`` and they
don't change if the database can't change).

Add document
------------

-  ``MSG_ADDDOCUMENT <serialised Xapian::Document object>``
-  ``REPLY_ADDDOCUMENT I<document id>``

Delete document
---------------

-  ``MSG_DELETEDOCUMENT I<document id>``
-  ``REPLY_DONE``

Delete document by term (39.0 compatibility)
--------------------------------------------

-  ``MSG_DELETEDOCUMENTTERM_ <term name>``

Delete document by term
-----------------------

-  ``MSG_DELETEDOCUMENTTERM <term name>``
-  ``REPLY_DONE``

Replace document (39.0 compatibility)
-------------------------------------

-  ``MSG_REPLACEDOCUMENT_ I<document id> <serialised Xapian::Document object>``

Replace document
----------------

-  ``MSG_REPLACEDOCUMENT I<document id> <serialised Xapian::Document object>``
-  ``REPLY_DONE``

Replace document by term
------------------------

-  ``MSG_REPLACEDOCUMENTTERM L<term name> <serialised Xapian::Document object>``
-  ``REPLY_ADDDOCUMENT I<document id>``

Cancel (39.0 compatibility)
---------------------------

-  ``MSG_CANCEL_``

Cancel
------

-  ``MSG_CANCEL``
-  ``REPLY_DONE``

Commit
------

-  ``MSG_COMMIT``
-  ``REPLY_DONE``

Set metadata (39.0 compatibility)
---------------------------------

-  ``MSG_SETMETADATA_ L<key> <value>``

Set metadata
------------

-  ``MSG_SETMETADATA L<key> <value>``
-  ``REPLY_DONE``

Get metadata
------------

-  ``MSG_GETMETADATA <key>``
-  ``REPLY_METADATA <value>``

Metadata keys
-------------

-  ``MSG_METADATAKEYLIST <prefix>``
-  ``REPLY_METADATAKEYLIST C<chars of previous key to reuse> <string to append>``
-  ``...``
-  ``REPLY_DONE``

Add spelling (39.0 compatibility)
---------------------------------

-  ``MSG_ADDSPELLING_ I<freqinc> <word>``

Add spelling
------------

-  ``MSG_ADDSPELLING I<freqinc> <word>``
-  ``REPLY_DONE``

Remove spelling
---------------

-  ``MSG_REMOVESPELLING I<freqdec> <word>``

