Quickstart
==========

The document contains a quick introduction to the basic concepts, and
then a walk-through development of a simple application using the Xapian
library, together with commentary on how the application could be taken
further. It deliberately avoids going into a lot of detail - see the
`rest of the documentation`_ for more detail.

.. _`rest of the documentation`: index.html

--------------

Requirements
------------

Before following the steps outlined in this document, you will need to
have the Xapian library installed on your system. For instructions on
obtaining and installing Xapian, read the `Installation <install.html>`_
document.

--------------

Databases
---------

An information retrieval system using Xapian typically has two parts.
The first part is the *indexer*, which takes documents in various
formats, processes them so that they can be efficiently searched, and
stores the processed documents in an appropriate data structure (the
*database*). The second part is the *searcher*, which takes queries and
reads the database to return a list of the documents relevant to each
query.

The database is the data structure which ties the indexer and searcher
together, and is fundamental to the retrieval process. Given how
fundamental it is, it is unsurprising that different applications put
different demands on the database. For example, some applications may be
happy to deal with searching a static collection of data, but need to do
this extremely fast (for example, a web search engine which builds new
databases from scratch nightly or even weekly). Other applications may
require that new data can be added to the system incrementally, but
don't require extremely high performance searching (perhaps an email
system, which is only being searched occasionally). There are many other
constraints which may be placed on an information retrieval system: for
example, it may be required to have small database sizes, even at the
expense of getting poorer results from the system.

To provide the required flexibility, Xapian has the ability to use one
of many available database *backends*, each of which satisfies a
different set of constraints, and stores its data in a different way.
Currently, these must be compiled into the whole system, and selected at
runtime, but the ability to dynamically load modules for each of these
backends is likely to be added in future, and would require little
design modification.

--------------

An example indexer
------------------

We now present sample code for an indexer. This is deliberately
simplified to make it easier to follow. You can also read it in an `HTML
formatted version`_.

.. _`HTML formatted version`: quickstartindex.cc.html

The "indexer" presented here is simply a small program which takes a
path to a database and a set of parameters defining a document on the
command line, and stores that document as a new entry in the database.

Include header files
~~~~~~~~~~~~~~~~~~~~

The first requirement in any program using the Xapian library is to
include the Xapian header file, "``xapian.h``":
::

        #include <xapian.h>

We're going to use C++ iostreams for output, so we need to include the
``iostream`` header, and we'll also import everything from namespace
``std`` for convenience:
::

        #include <iostream>
        using namespace std;

Our example only has a single function, ``main()``, so next we define
that:
::

        int main(int argc, char **argv)

Options parsing
~~~~~~~~~~~~~~~

For this example we do very simple options parsing. We are going to use
the core functionality of Xapian of searching for specific terms in the
database, and we are not going to use any of the extra facilities, such
as the keys which may be associated with each document. We are also
going to store a simple string as the data associated with each
document.

Thus, our command line syntax is:

-  **Parameter 1** - the (possibly relative) path to the database.
-  **Parameter 2** - the string to be stored as the document data.
-  **Parameters 3 onward** - the terms to be stored in the database. The
   terms will be assumed to occur at successive positions in the
   document.

The validity of a command line can therefore be checked very simply by
ensuring that there are at least 3 parameters:
::

        if (argc < 4) {
            cout << "usage: " << argv[0] <<
                    " <path to database> <document data> <document terms>" << endl;
            exit(1);
        }

Catching exceptions
~~~~~~~~~~~~~~~~~~~

When an error occurs in Xapian it is reported by means of the C++
exception mechanism. All errors in Xapian are derived classes of
``Xapian::Error``, so simple error handling can be performed by
enclosing all the code in a try-catch block to catch any
``Xapian::Error`` exceptions. A (hopefully) helpful message can be
extracted from the ``Xapian::Error`` object by calling its ``get_msg()``
method, which returns a human readable string.

Note that all calls to the Xapian library should be performed inside a
try-catch block, since otherwise errors will result in uncaught
exceptions; this usually results in the execution aborting.

Note also that Xapian::Error is a virtual base class, and thus can't be
copied: you must therefore catch exceptions by reference, as in the
following example code:

::

        try {
            [code which accesses Xapian]
        } catch (const Xapian::Error & error) {
            cout << "Exception: " << error.get_msg() << endl;
        }

Opening the database
~~~~~~~~~~~~~~~~~~~~

In Xapian, a database is opened for writing by creating a
Xapian::WritableDatabase object.

If you pass Xapian::DB\_CREATE\_OR\_OPEN and there isn't an existing
database in the specified directory, Xapian will try to create a new
empty database there. If there is already database in the specified
directory, it will be opened.

If an error occurs when trying to open a database, or to create a new
database, an exception, usually of type ``Xapian::DatabaseOpeningError``
or ``Xapian::DatabaseCreateError``, will be thrown.

The code to open a database for writing is, then:

::

        Xapian::WritableDatabase database(argv[1], Xapian::DB_CREATE_OR_OPEN);

Preparing the new document
~~~~~~~~~~~~~~~~~~~~~~~~~~

Now that we have the database open, we need to prepare a document to put
in it. This is done by creating a Xapian::Document object, filling this
with data, and then giving it to the database.

The first step, then, is to create the document:

::

        Xapian::Document newdocument;

Each ``Xapian::Document`` has a "cargo" known as the *document data*.
This data is opaque to Xapian - the meaning of it is entirely
user-defined. Typically it contains information to allow results to be
displayed by the application, for example a URL for the indexed document
and some text which is to be displayed when returning the document as
search result.

For our example, we shall simply store the second parameter given on the
command line in the data field:

::

        newdocument.set_data(string(argv[2]));

The next step is to put the terms which are to be used when searching
for the document into the Xapian::Document object.

We shall use the ``add_posting()`` method, which adds an occurrence of a
term to the struct. The first parameter is the "*termname*", which is a
string defining the term. This string can be anything, as long as the
same string is always used to refer to the same term. The string will
often be the (possibly stemmed) text of the term, but might be in a
compressed, or even hashed, form. Most backends impose a limit on the
length of a termname (for chert the limit is 245 bytes).

The second parameter is the position at which the term occurs within the
document. These positions start at 1. This information is used for some
search features such as phrase matching or passage retrieval, but is not
essential to the search.

We add postings for terms with the termname given as each of the
remaining command line parameters:

::

        for (int i = 3; i < argc; ++i) {
            newdocument.add_posting(argv[i], i - 2);
        }

Adding the document to the database
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Finally, we can add the document to the database. This simply involves
calling ``Xapian::WritableDatabase::add_document()``, and passing it the
``Xapian::Document`` object:

::

        database.add_document(newdocument);

The operation of adding a document is atomic: either the document will
be added, or an exception will be thrown and the document will not be in
the new database.

``add_document()`` returns a value of type ``Xapian::docid``. This is
the document ID of the newly added document, which is simply a handle
which can be used to access the document in future.

Note that this use of ``add_document()`` is actually fairly inefficient:
if we had a large database, it would be desirable to group as many
document additions together as possible, by encapsulating them within a
session. For details of this, and of the transaction facility for
performing sets of database modifications atomically, see the `API
Overview <overview.html>`_.

--------------

An example searcher
-------------------

Now we show the code for a simple searcher, which will search the
database built by the indexer above. Again, you can read `an HTML
formatted version <quickstartsearch.cc.html>`_.

The "searcher" presented here is, like the "indexer", simply a small
command line driven program. It takes a path to a database and some
search terms, performs a probabilistic search for documents represented
by those terms and displays a ranked list of matching documents.

Setting up
~~~~~~~~~~

Just like "quickstartindex", we have a single-function example. So we
include the Xapian header file, and begin:

::

        #include <xapian.h>

        int main(int argc, char **argv)
        {

Options parsing
~~~~~~~~~~~~~~~

Again, we are going to use no special options, and have a very simple
command line syntax:

-  **Parameter 1** - the (possibly relative) path to the database.
-  **Parameters 2 onward** - the terms to be searched for in the
   database.

The validity of a command line can therefore be checked very simply by
ensuring that there are at least 2 parameters:

::

        if (argc < 3) {
            cout << "usage: " << argv[0] <<
                    " <path to database> <search terms>" << endl;
            exit(1);
        }

Catching exceptions
~~~~~~~~~~~~~~~~~~~

Again, this is performed just as it was for the simple indexer.

::

        try {
            [code which accesses Xapian]
        } catch (const Xapian::Error & error) {
            cout << "Exception: " << error.get_msg() << endl;
        }

Specifying the databases
~~~~~~~~~~~~~~~~~~~~~~~~

Xapian has the ability to search over many databases simultaneously,
possibly even with the databases distributed across a network of
machines. Each database can be in its own format, so, for example, we
might have a system searching across two remote databases and a flint
database.

To open a single database, we create a Xapian::Database object, passing
the path to the database we want to open:

::

        Xapian::Database db(argv[1]);

You can also search multiple database by adding them together using
``Xapian::Database::add_database``:

::

        Xapian::Database databases;
        databases.add_database(Xapian::Database(argv[1]));
        databases.add_database(Xapian::Database(argv[2]));

Starting an enquire session
~~~~~~~~~~~~~~~~~~~~~~~~~~~

All searches across databases by Xapian are performed within the context
of an "*Enquire*" session. This session is represented by a
``Xapian::Enquire`` object, and is across a specified collection of
databases. To change the database collection, it is necessary to open a
new enquire session, by creating a new ``Xapian::Enquire`` object.
::

        Xapian::Enquire enquire(databases);

An enquire session is also the context within which all other database
reading operations, such as query expansion and reading the data
associated with a document, are performed.

Preparing to search
~~~~~~~~~~~~~~~~~~~

We are going to use all command line parameters from the second onward
as terms to search for in the database. For convenience, we shall store
them in an STL vector. This is probably the point at which we would want
to apply a stemming algorithm, or any other desired normalisation and
conversion operation, to the terms.
::

        vector<string> queryterms;
        for (int optpos = 2; optpos < argc; optpos++) {
            queryterms.push_back(argv[optpos]);
        }

Queries are represented within Xapian by ``Xapian::Query`` objects, so
the next step is to construct one from our query terms. Conveniently
there is a constructor which will take our vector of terms and create an
``Xapian::Query`` object from it.
::

        Xapian::Query query(Xapian::Query::OP_OR, queryterms.begin(), queryterms.end());

You will notice that we had to specify an operation to be performed on
the terms (the ``Xapian::Query::OP_OR`` parameter). Queries in Xapian
are actually fairly complex things: a full range of boolean operations
can be applied to queries to restrict the result set, and probabilistic
weightings are then applied to order the results by relevance. By
specifying the OR operation, we are not performing any boolean
restriction, and are performing a traditional pure probabilistic search.

We now print a message out to confirm to the user what the query being
performed is. This is done with the ``Xapian::Query::get_description()``
method, which is mainly included for debugging purposes, and displays a
string representation of the query.

::

        cout << "Performing query `" <<
             query.get_description() << "'" << endl;

Performing the search
~~~~~~~~~~~~~~~~~~~~~

Now, we are ready to perform the search. The first step of this is to
give the query object to the enquire session::

        enquire.set_query(query);

Next, we ask for the results of the search, which implicitly performs the
the search.  We use the ``get_mset()`` method to get the results, which are
returned in an ``Xapian::MSet`` object. (MSet for Match Set)

``get_mset()`` can take many parameters, such as a set of relevant
documents to use, and various options to modify the search, but we give
it the minimum, which is the first document to return (starting at 0 for
the top ranked document), and the maximum number of documents to return
(we specify 10 here)::

        Xapian::MSet matches = enquire.get_mset(0, 10);

Displaying the results of the search
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Finally, we display the results of the search. The results are stored in
in the ``Xapian::MSet`` object, which provides the features required to
be an STL-compatible container, so first we display how many items are
in the MSet::

        cout << matches.size() << " results found" << endl;

Now we display some information about each of the items in the
``Xapian::MSet``. We access these items using an
``Xapian::MSetIterator``:

-  First, we display the document ID, accessed by ``*i``. This is not
   usually very useful information to give to users, but it is at least
   a unique handle on each document.
-  Next, we display a "percentage" score for the document. Readers
   familiar with Information Retrieval will not be surprised to hear
   that this is not really a percentage: it is just a value from 0 to
   100, such that a more relevant document has a higher value. We get
   this using ``i.get_percent()``.
-  Last, we display the data associated with each returned document,
   which was specified by the user at database generation time. To do
   this, we first use ``i.get_document()`` to get an
   ``Xapian::Document`` object representing the returned document; then
   we use the ``get_data()`` method of this object to get access to the
   data stored in this document.

::

        Xapian::MSetIterator i;
        for (i = matches.begin(); i != matches.end(); ++i) {
            cout << "Document ID " << *i << "\t";
            cout << i.get_percent() << "% ";
            Xapian::Document doc = i.get_document();
            cout << "[" << doc.get_data() << "]" << endl;
        }

--------------

Compiling
---------

Now that we have the code written, all we need to do is compile it!

Finding the Xapian library
~~~~~~~~~~~~~~~~~~~~~~~~~~

A small utility, "xapian-config", is installed along with Xapian to
assist you in finding the installed Xapian library, and in generating
the flags to pass to the compiler and linker to compile.

After a successful compilation, this utility should be in your path, so
you can simply run
::

    xapian-config --cxxflags

to determine the flags to pass to the compiler, and
::

    xapian-config --libs

to determine the flags to pass to the linker. These flags are returned
on the utility's standard output (so you could use backtick notation to
include them on your command line).

If your project uses the GNU autoconf tool, you may also use the
``XO_LIB_XAPIAN`` macro, which is included as part of Xapian, and will
check for an installation of Xapian and set (and ``AC_SUBST``) the
``XAPIAN_CXXFLAGS`` and ``XAPIAN_LIBS`` variables to be the flags to
pass to the compiler and linker, respectively.

If you don't use GNU autoconf, don't worry about this.

Compiling the quickstart examples
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Once you know the compilation flags, compilation is a simple matter of
invoking the compiler! For our example, we could compile the two
utilities (quickstartindex and quickstartsearch) with the commands::

    c++ `xapian-config --cxxflags` quickstartindex.cc `xapian-config --libs` -o quickstartindex
    c++ `xapian-config --cxxflags` quickstartsearch.cc `xapian-config --libs` -o quickstartsearch

--------------

Running the examples
--------------------

Once we have compiled the above examples, we can build up a simple
database as follows.
::

    $ ./quickstartindex proverbs \
    > "people who live in glass houses should not throw stones" \
    > people live glass house stone
    $ ./quickstartindex proverbs \
    > "Don't look a gift horse in the mouth" \
    > look gift horse mouth

For the first command, the database directory doesn't already exist, so
Xapian will create it and also create the database files inside it.  For
the second command, it will use the database which now exists, so
we should now have a database with a couple of documents in it. Looking
in the database directory, you should see something like:
::

    $ ls proverbs/
    [some files]

Given the small amount of data in the database, you may be concerned
that the total size of these files is a little over 32KB. Be reassured
that the database is block structured, here consisting of largely empty
blocks, and will behave much better for large databases.

We can now perform searches over the database using the quickstartsearch
program.
::

    $ ./quickstartsearch proverbs look
    Performing query `look'
    1 results found
    Document ID 2   50% [Don't look a gift horse in the mouth]

