
// File: index.xml

// File: classXapian_1_1BM25Weight.xml
%feature("docstring") Xapian::BM25Weight "

BM25 weighting scheme.

BM25 weighting options : The BM25 formula is \\\\[
\\\\frac{k_{2}.n_{q}}{1+L_{d}}+\\\\sum_{t}\\\\frac{(k_{3}+1)q_{t
}}{k_{3}+q_{t}}.\\\\frac{(k_{1}+1)f_{t,d}}{k_{1}((1-b)+bL_{d})+f_{t,d}
}.w_{t} \\\\] where  $w_{t}$ is the termweight of term t

$f_{t,d}$ is the within document frequency of term t in document d

$q_{t}$ is the within query frequency of term t

$L_{d}$ is the normalised length of document d

$n_{q}$ is the size of the query

$k_{1}$, $k_{2}$, $k_{3}$ and $b$ are user specified parameters ";

%feature("docstring")  Xapian::BM25Weight::BM25Weight "

Construct a BM25 weight.

Xapian::BM25Weight::BM25Weight(double k1_, double k2_, double k3_,
double b_, double min_normlen_)

Parameters:
-----------

k1:  governs the importance of within document frequency. Must be >=
0. 0 means ignore wdf. Default is 1.

k2:  compensation factor for the high wdf values in large documents.
Must be >= 0. 0 means no compensation. Default is 0.

k3:  governs the importance of within query frequency. Must be >= 0. 0
means ignore wqf. Default is 1.

b:  Relative importance of within document frequency and document
length. Must be >= 0 and <= 1. Default is 0.5.

min_normlen:  specifies a cutoff on the minimum value that can be used
for a normalised document length - smaller values will be forced up to
this cutoff. This prevents very small documents getting a huge bonus
weight. Default is 0.5. ";

%feature("docstring")  Xapian::BM25Weight::BM25Weight "Xapian::BM25Weight::BM25Weight() ";

%feature("docstring")  Xapian::BM25Weight::clone "

Return a new weight object of this type.

BM25Weight* Xapian::BM25Weight::clone() const

A subclass called FooWeight taking parameters param1 and param2 should
implement this as:

virtual FooWeight * clone() const { return new FooWeight(param1,
param2); } ";

%feature("docstring")  Xapian::BM25Weight::~BM25Weight "Xapian::BM25Weight::~BM25Weight() ";

%feature("docstring")  Xapian::BM25Weight::name "

Name of the weighting scheme.

std::string Xapian::BM25Weight::name() const

If the subclass is called FooWeight, this should return \"Foo\". ";

%feature("docstring")  Xapian::BM25Weight::serialise "

Serialise object parameters into a string.

std::string Xapian::BM25Weight::serialise() const ";

%feature("docstring")  Xapian::BM25Weight::unserialise "

Create object given string serialisation returned by serialise().

BM25Weight* Xapian::BM25Weight::unserialise(const std::string &s)
const ";

%feature("docstring")  Xapian::BM25Weight::get_sumpart "

Get a weight which is part of the sum over terms being performed.

Xapian::weight Xapian::BM25Weight::get_sumpart(Xapian::termcount wdf,
Xapian::doclength len) const

This returns a weight for a given term and document. These weights are
summed to give a total weight for the document.

Parameters:
-----------

wdf:  the within document frequency of the term.

len:  the (unnormalised) document length. ";

%feature("docstring")  Xapian::BM25Weight::get_maxpart "

Gets the maximum value that get_sumpart() may return.

Xapian::weight Xapian::BM25Weight::get_maxpart() const

This is used in optimising searches, by having the postlist tree decay
appropriately when parts of it can have limited, or no, further
effect. ";

%feature("docstring")  Xapian::BM25Weight::get_sumextra "

Get an extra weight for a document to add to the sum calculated over
the query terms.

Xapian::weight Xapian::BM25Weight::get_sumextra(Xapian::doclength len)
const

This returns a weight for a given document, and is used by some
weighting schemes to account for influence such as document length.

Parameters:
-----------

len:  the (unnormalised) document length. ";

%feature("docstring")  Xapian::BM25Weight::get_maxextra "

Gets the maximum value that get_sumextra() may return.

Xapian::weight Xapian::BM25Weight::get_maxextra() const

This is used in optimising searches. ";

%feature("docstring")  Xapian::BM25Weight::get_sumpart_needs_doclength
"

return false if the weight object doesn't need doclength

bool Xapian::BM25Weight::get_sumpart_needs_doclength() const ";


// File: classXapian_1_1BoolWeight.xml
%feature("docstring") Xapian::BoolWeight "

Boolean weighting scheme (everything gets 0). ";

%feature("docstring")  Xapian::BoolWeight::clone "

Return a new weight object of this type.

BoolWeight* Xapian::BoolWeight::clone() const

A subclass called FooWeight taking parameters param1 and param2 should
implement this as:

virtual FooWeight * clone() const { return new FooWeight(param1,
param2); } ";

%feature("docstring")  Xapian::BoolWeight::BoolWeight "Xapian::BoolWeight::BoolWeight() ";

%feature("docstring")  Xapian::BoolWeight::~BoolWeight "Xapian::BoolWeight::~BoolWeight() ";

%feature("docstring")  Xapian::BoolWeight::name "

Name of the weighting scheme.

std::string Xapian::BoolWeight::name() const

If the subclass is called FooWeight, this should return \"Foo\". ";

%feature("docstring")  Xapian::BoolWeight::serialise "

Serialise object parameters into a string.

std::string Xapian::BoolWeight::serialise() const ";

%feature("docstring")  Xapian::BoolWeight::unserialise "

Create object given string serialisation returned by serialise().

BoolWeight* Xapian::BoolWeight::unserialise(const std::string &s)
const ";

%feature("docstring")  Xapian::BoolWeight::get_sumpart "

Get a weight which is part of the sum over terms being performed.

Xapian::weight Xapian::BoolWeight::get_sumpart(Xapian::termcount wdf,
Xapian::doclength len) const

This returns a weight for a given term and document. These weights are
summed to give a total weight for the document.

Parameters:
-----------

wdf:  the within document frequency of the term.

len:  the (unnormalised) document length. ";

%feature("docstring")  Xapian::BoolWeight::get_maxpart "

Gets the maximum value that get_sumpart() may return.

Xapian::weight Xapian::BoolWeight::get_maxpart() const

This is used in optimising searches, by having the postlist tree decay
appropriately when parts of it can have limited, or no, further
effect. ";

%feature("docstring")  Xapian::BoolWeight::get_sumextra "

Get an extra weight for a document to add to the sum calculated over
the query terms.

Xapian::weight Xapian::BoolWeight::get_sumextra(Xapian::doclength len)
const

This returns a weight for a given document, and is used by some
weighting schemes to account for influence such as document length.

Parameters:
-----------

len:  the (unnormalised) document length. ";

%feature("docstring")  Xapian::BoolWeight::get_maxextra "

Gets the maximum value that get_sumextra() may return.

Xapian::weight Xapian::BoolWeight::get_maxextra() const

This is used in optimising searches. ";

%feature("docstring")  Xapian::BoolWeight::get_sumpart_needs_doclength
"

return false if the weight object doesn't need doclength

bool Xapian::BoolWeight::get_sumpart_needs_doclength() const ";


// File: classXapian_1_1Database.xml
%feature("docstring") Xapian::Database "

This class is used to access a database, or a group of databases.

For searching, this class is used in conjunction with an Enquire
object.

Parameters:
-----------

InvalidArgumentError:  will be thrown if an invalid argument is
supplied, for example, an unknown database type.

DatabaseOpeningError:  may be thrown if the database cannot be opened
(for example, a required file cannot be found). ";

%feature("docstring")  Xapian::Database::add_database "

Add an existing database (or group of databases) to those accessed by
this object.

void Xapian::Database::add_database(const Database &database)

Parameters:
-----------

database:  the database(s) to add. ";

%feature("docstring")  Xapian::Database::Database "

Create a Database with no databases in.

Xapian::Database::Database() ";

%feature("docstring")  Xapian::Database::Database "

Open a Database, automatically determining the database backend to
use.

Xapian::Database::Database(const std::string &path)

Parameters:
-----------

path:  directory that the database is stored in. ";

%feature("docstring")  Xapian::Database::Database "Xapian::Database::Database(Internal *internal) ";

%feature("docstring")  Xapian::Database::~Database "

Destroy this handle on the database.

virtual Xapian::Database::~Database()

If there are no copies of this object remaining, the database(s) will
be closed. ";

%feature("docstring")  Xapian::Database::Database "

Copying is allowed.

Xapian::Database::Database(const Database &other)

The internals are reference counted, so copying is cheap. ";

%feature("docstring")  Xapian::Database::reopen "

Re-open the database.

void Xapian::Database::reopen()

This re-opens the database(s) to the latest available version(s). It
can be used either to make sure the latest results are returned, or to
recover from a Xapian::DatabaseModifiedError. ";

%feature("docstring")  Xapian::Database::get_description "

Introspection method.

virtual std::string Xapian::Database::get_description() const

A string describing this object. ";

%feature("docstring")  Xapian::Database::postlist_begin "

An iterator pointing to the start of the postlist for a given term.

PostingIterator Xapian::Database::postlist_begin(const std::string
&tname) const

If the term name is the empty string, the iterator returned will list
all the documents in the database. Such an iterator will always return
a WDF value of 1, since there is no obvious meaning for this quantity
in this case. ";

%feature("docstring")  Xapian::Database::postlist_end "

Corresponding end iterator to postlist_begin().

PostingIterator Xapian::Database::postlist_end(const std::string &)
const ";

%feature("docstring")  Xapian::Database::termlist_begin "

An iterator pointing to the start of the termlist for a given
document.

TermIterator Xapian::Database::termlist_begin(Xapian::docid did) const
";

%feature("docstring")  Xapian::Database::termlist_end "

Corresponding end iterator to termlist_begin().

TermIterator Xapian::Database::termlist_end(Xapian::docid) const ";

%feature("docstring")  Xapian::Database::has_positions "

Does this database have any positional information?

bool Xapian::Database::has_positions() const ";

%feature("docstring")  Xapian::Database::positionlist_begin "

An iterator pointing to the start of the position list for a given
term in a given document.

PositionIterator Xapian::Database::positionlist_begin(Xapian::docid
did, const std::string &tname) const ";

%feature("docstring")  Xapian::Database::positionlist_end "

Corresponding end iterator to positionlist_begin().

PositionIterator Xapian::Database::positionlist_end(Xapian::docid,
const std::string &) const ";

%feature("docstring")  Xapian::Database::allterms_begin "

An iterator which runs across all terms in the database.

TermIterator Xapian::Database::allterms_begin() const ";

%feature("docstring")  Xapian::Database::allterms_end "

Corresponding end iterator to allterms_begin().

TermIterator Xapian::Database::allterms_end() const ";

%feature("docstring")  Xapian::Database::get_doccount "

Get the number of documents in the database.

Xapian::doccount Xapian::Database::get_doccount() const ";

%feature("docstring")  Xapian::Database::get_lastdocid "

Get the highest document id which has been used in the database.

Xapian::docid Xapian::Database::get_lastdocid() const ";

%feature("docstring")  Xapian::Database::get_avlength "

Get the average length of the documents in the database.

Xapian::doclength Xapian::Database::get_avlength() const ";

%feature("docstring")  Xapian::Database::get_termfreq "

Get the number of documents in the database indexed by a given term.

Xapian::doccount Xapian::Database::get_termfreq(const std::string
&tname) const ";

%feature("docstring")  Xapian::Database::term_exists "

Check if a given term exists in the database.

bool Xapian::Database::term_exists(const std::string &tname) const

Return true if and only if the term exists in the database. This is
the same as (get_termfreq(tname) != 0), but will often be more
efficient. ";

%feature("docstring")  Xapian::Database::get_collection_freq "

Return the total number of occurrences of the given term.

Xapian::termcount Xapian::Database::get_collection_freq(const
std::string &tname) const

This is the sum of the number of ocurrences of the term in each
document it indexes: ie, the sum of the within document frequencies of
the term.

Parameters:
-----------

tname:  The term whose collection frequency is being requested. ";

%feature("docstring")  Xapian::Database::get_doclength "

Get the length of a document.

Xapian::doclength Xapian::Database::get_doclength(Xapian::docid did)
const ";

%feature("docstring")  Xapian::Database::keep_alive "

Send a \"keep-alive\" to remote databases to stop them timing out.

void Xapian::Database::keep_alive() ";

%feature("docstring")  Xapian::Database::get_document "

Get a document from the database, given its document id.

Xapian::Document Xapian::Database::get_document(Xapian::docid did)
const

This method returns a Xapian::Document object which provides the
information about a document.

Parameters:
-----------

did:  The document id for which to retrieve the data.

A Xapian::Document object containing the document data

Parameters:
-----------

Xapian::DocNotFoundError:  The document specified could not be found
in the database. ";


// File: classXapian_1_1Document.xml
%feature("docstring") Xapian::Document "

A document in the database - holds data, values, terms, and postings.
";

%feature("docstring")  Xapian::Document::Document "Xapian::Document::Document(Internal *internal_) ";

%feature("docstring")  Xapian::Document::Document "

Copying is allowed.

Xapian::Document::Document(const Document &other)

The internals are reference counted, so copying is cheap. ";

%feature("docstring")  Xapian::Document::Document "

Make a new empty Document.

Xapian::Document::Document() ";

%feature("docstring")  Xapian::Document::~Document "

Destructor.

Xapian::Document::~Document() ";

%feature("docstring")  Xapian::Document::get_value "

Get value by number.

std::string Xapian::Document::get_value(Xapian::valueno value) const

Returns an empty string if no value with the given number is present
in the document.

Parameters:
-----------

value:  The number of the value. ";

%feature("docstring")  Xapian::Document::add_value "

Add a new value.

void Xapian::Document::add_value(Xapian::valueno valueno, const
std::string &value)

It will replace any existing value with the same number. ";

%feature("docstring")  Xapian::Document::remove_value "

Remove any value with the given number.

void Xapian::Document::remove_value(Xapian::valueno valueno) ";

%feature("docstring")  Xapian::Document::clear_values "

Remove all values associated with the document.

void Xapian::Document::clear_values() ";

%feature("docstring")  Xapian::Document::get_data "

Get data stored in the document.

std::string Xapian::Document::get_data() const

This is a potentially expensive operation, and shouldn't normally be
used in a match decider functor. Put data for use by match deciders in
a value instead. ";

%feature("docstring")  Xapian::Document::set_data "

Set data stored in the document.

void Xapian::Document::set_data(const std::string &data) ";

%feature("docstring")  Xapian::Document::add_posting "

Add an occurrence of a term at a particular position.

void Xapian::Document::add_posting(const std::string &tname,
Xapian::termpos tpos, Xapian::termcount wdfinc=1)

Multiple occurrences of the term at the same position are represented
only once in the positional information, but do increase the wdf.

If the term is not already in the document, it will be added to it.

Parameters:
-----------

tname:  The name of the term.

tpos:  The position of the term.

wdfinc:  The increment that will be applied to the wdf for this term.
";

%feature("docstring")  Xapian::Document::add_term "

Add a term to the document, without positional information.

void Xapian::Document::add_term(const std::string &tname,
Xapian::termcount wdfinc=1)

Any existing positional information for the term will be left
unmodified.

Parameters:
-----------

tname:  The name of the term.

wdfinc:  The increment that will be applied to the wdf for this term.
";

%feature("docstring")  Xapian::Document::XAPIAN_DEPRECATED "

Old name for add_term().

Xapian::Document::XAPIAN_DEPRECATED(void add_term_nopos(const
std::string &term, Xapian::termcount wdfinc=1))

Deprecated This method is deprecated and present only for backward
compatibility. Use add_term() instead. ";

%feature("docstring")  Xapian::Document::remove_posting "

Remove a posting of a term from the document.

void Xapian::Document::remove_posting(const std::string &tname,
Xapian::termpos tpos, Xapian::termcount wdfdec=1)

Note that the term will still index the document even if all
occurrences are removed. To remove a term from a document completely,
use remove_term().

Parameters:
-----------

tname:  The name of the term.

tpos:  The position of the term.

wdfdec:  The decrement that will be applied to the wdf when removing
this posting. The wdf will not go below the value of 0.

Parameters:
-----------

Xapian::InvalidArgumentError:  will be thrown if the term is not at
the position specified in the position list for this term in this
document.

Xapian::InvalidArgumentError:  will be thrown if the term is not in
the document ";

%feature("docstring")  Xapian::Document::remove_term "

Remove a term and all postings associated with it.

void Xapian::Document::remove_term(const std::string &tname)

Parameters:
-----------

tname:  The name of the term.

Parameters:
-----------

Xapian::InvalidArgumentError:  will be thrown if the term is not in
the document ";

%feature("docstring")  Xapian::Document::clear_terms "

Remove all terms (and postings) from the document.

void Xapian::Document::clear_terms() ";

%feature("docstring")  Xapian::Document::termlist_count "

Count the terms in this document.

Xapian::termcount Xapian::Document::termlist_count() const ";

%feature("docstring")  Xapian::Document::termlist_begin "

Iterator for the terms in this document.

TermIterator Xapian::Document::termlist_begin() const ";

%feature("docstring")  Xapian::Document::termlist_end "

Equivalent end iterator for termlist_begin().

TermIterator Xapian::Document::termlist_end() const ";

%feature("docstring")  Xapian::Document::values_count "

Count the values in this document.

Xapian::termcount Xapian::Document::values_count() const ";

%feature("docstring")  Xapian::Document::values_begin "

Iterator for the values in this document.

ValueIterator Xapian::Document::values_begin() const ";

%feature("docstring")  Xapian::Document::values_end "

Equivalent end iterator for values_begin().

ValueIterator Xapian::Document::values_end() const ";

%feature("docstring")  Xapian::Document::get_description "

Introspection method.

std::string Xapian::Document::get_description() const

A string representing this Document. ";


// File: classXapian_1_1Enquire.xml
%feature("docstring") Xapian::Enquire "

This class provides an interface to the information retrieval system
for the purpose of searching.

Databases are usually opened lazily, so exceptions may not be thrown
where you would expect them to be. You should catch Xapian::Error
exceptions when calling any method in Xapian::Enquire.

Parameters:
-----------

Xapian::InvalidArgumentError:  will be thrown if an invalid argument
is supplied, for example, an unknown database type. ";

%feature("docstring")  Xapian::Enquire::Enquire "

Create a Xapian::Enquire object.

Xapian::Enquire::Enquire(const Database &database, ErrorHandler
*errorhandler_=0)

This specification cannot be changed once the Xapian::Enquire is
opened: you must create a new Xapian::Enquire object to access a
different database, or set of databases.

Parameters:
-----------

database:  Specification of the database or databases to use.

errorhandler_:  A pointer to the error handler to use. Ownership of
the object pointed to is not assumed by the Xapian::Enquire object -
the user should delete the Xapian::ErrorHandler object after the
Xapian::Enquire object is deleted. To use no error handler, this
parameter should be 0. ";

%feature("docstring")  Xapian::Enquire::~Enquire "

Close the Xapian::Enquire object.

Xapian::Enquire::~Enquire() ";

%feature("docstring")  Xapian::Enquire::set_query "

Set the query to run.

void Xapian::Enquire::set_query(const Xapian::Query &query,
Xapian::termcount qlen=0)

Parameters:
-----------

query:  the new query to run.

qlen:  the query length to use in weight calculations - by default the
sum of the wqf of all terms is used. ";

%feature("docstring")  Xapian::Enquire::get_query "

Get the query which has been set.

const Xapian::Query& Xapian::Enquire::get_query() const

This is only valid after set_query() has been called.

Parameters:
-----------

Xapian::InvalidArgumentError:  will be thrown if query has not yet
been set. ";

%feature("docstring")  Xapian::Enquire::set_weighting_scheme "

Set the weighting scheme to use for queries.

void Xapian::Enquire::set_weighting_scheme(const Weight &weight_)

Parameters:
-----------

weight_:  the new weighting scheme. If no weighting scheme is
specified, the default is BM25 with the default parameters. ";

%feature("docstring")  Xapian::Enquire::set_collapse_key "

Set the collapse key to use for queries.

void Xapian::Enquire::set_collapse_key(Xapian::valueno collapse_key)

Parameters:
-----------

collapse_key:  value number to collapse on - at most one mset entry
with each particular value will be returned.

The entry returned will be the best entry with that particular value
(highest weight or highest sorting key).

An example use might be to create a value for each document containing
an MD5 hash of the document contents. Then duplicate documents from
different sources can be eliminated at search time (it's better to
eliminate duplicates at index time, but this may not be always be
possible - for example the search may be over more than one Xapian
database).

Another use is to group matches in a particular category (e.g. you
might collapse a mailing list search on the Subject: so that there's
only one result per discussion thread). In this case you can use
get_collapse_count() to give the user some idea how many other results
there are. And if you index the Subject: as a boolean term as well as
putting it in a value, you can offer a link to a non-collapsed search
restricted to that thread using a boolean filter.

(default is Xapian::BAD_VALUENO which means no collapsing). ";

%feature("docstring")  Xapian::Enquire::set_docid_order "

Set the direction in which documents are ordered by document id in the
returned MSet.

void Xapian::Enquire::set_docid_order(docid_order order)

This order only has an effect on documents which would otherwise have
equal rank. For a weighted probabilistic match with no sort value,
this means documents with equal weight. For a boolean match, with no
sort value, this means all documents. And if a sort value is used,
this means documents with equal sort value (and also equal weight if
ordering on relevance after the sort).

Parameters:
-----------

order:  This can be: Xapian::Enquire::ASCENDING docids sort in
ascending order (default)

Xapian::Enquire::DESCENDING docids sort in descending order

Xapian::Enquire::DONT_CARE docids sort in whatever order is most
efficient for the backend

Note: If you add documents in strict date order, then a boolean search
- i.e. set_weighting_scheme(Xapian::BoolWeight()) - with
set_docid_order(Xapian::Enquire::DESCENDING) is a very efficient way
to perform \"sort by date, newest first\". ";

%feature("docstring")  Xapian::Enquire::XAPIAN_DEPRECATED "

For compatibility with Xapian 0.8.5 and earlier.

Xapian::Enquire::XAPIAN_DEPRECATED(void set_sort_forward(bool
sort_forward))

Deprecated This method is now deprecated, use set_docid_order()
instead - set_sort_forward(true) -> set_docid_order(ASCENDING) and
set_sort_forward(false) -> set_docid_order(DESCENDING). ";

%feature("docstring")  Xapian::Enquire::set_cutoff "

Set the percentage and/or weight cutoffs.

void Xapian::Enquire::set_cutoff(Xapian::percent percent_cutoff,
Xapian::weight weight_cutoff=0)

Parameters:
-----------

percent_cutoff:  Minimum percentage score for returned documents. If a
document has a lower percentage score than this, it will not appear in
the mset. If your intention is to return only matches which contain
all the terms in the query, then it's more efficient to use
Xapian::Query::OP_AND instead of Xapian::Query::OP_OR in the query
than to use set_cutoff(100). (default 0 => no percentage cut-off).

weight_cutoff:  Minimum weight for a document to be returned. If a
document has a lower score that this, it will not appear in the mset.
It is usually only possible to choose an appropriate weight for cutoff
based on the results of a previous run of the same query; this is thus
mainly useful for alerting operations. The other potential use is with
a user specified weighting scheme. (default 0 => no weight cut-off).
";

%feature("docstring")  Xapian::Enquire::XAPIAN_DEPRECATED "

For compatibility with Xapian 0.8.5 and earlier.

Xapian::Enquire::XAPIAN_DEPRECATED(void set_sorting(Xapian::valueno
sort_key, int sort_bands, bool sort_by_relevance=false))

Deprecated This method is now deprecated, use set_sort_by_relevance(),
set_sort_by_value(), or set_sort_by_value_then_relevance() instead.
set_sorting(KEY, 1) -> set_sort_by_value(KEY)

set_sorting(KEY, 1, false) -> set_sort_by_value(KEY)

set_sorting(KEY, 1, true) -> set_sort_by_value_then_relevance(KEY)

set_sorting(ANYTHING, 0) -> set_sort_by_relevance()

set_sorting(Xapian::BAD_VALUENO, ANYTHING) -> set_sort_by_relevance()
";

%feature("docstring")  Xapian::Enquire::set_sort_by_relevance "

Set the sorting to be by relevance only.

void Xapian::Enquire::set_sort_by_relevance()

This is the default. ";

%feature("docstring")  Xapian::Enquire::set_sort_by_value "

Set the sorting to be by value only.

void Xapian::Enquire::set_sort_by_value(Xapian::valueno sort_key, bool
ascending=true)

Parameters:
-----------

sort_key:  value number to reorder on. Sorting is with a string
compare. If ascending is true (the default) higher is better; if
ascending is false, lower is better.

ascending:  If true, documents values which sort higher by string
compare are better. If false, the sort order is reversed. (default
true) ";

%feature("docstring")
Xapian::Enquire::set_sort_by_value_then_relevance "

Set the sorting to be by value, then by relevance for documents with
the same value.

void Xapian::Enquire::set_sort_by_value_then_relevance(Xapian::valueno
sort_key, bool ascending=true)

Parameters:
-----------

sort_key:  value number to reorder on. Sorting is with a string
compare. If ascending is true (the default) higher is better; if
ascending is false, lower is better.

ascending:  If true, documents values which sort higher by string
compare are better. If false, the sort order is reversed. (default
true) ";

%feature("docstring")
Xapian::Enquire::set_sort_by_relevance_then_value "

Set the sorting to be by relevance then value.

void Xapian::Enquire::set_sort_by_relevance_then_value(Xapian::valueno
sort_key, bool ascending=true)

Note that with the default BM25 weighting scheme parameters, non-
identical documents will rarely have the same weight, so this setting
will give very similar results to set_sort_by_relevance(). It becomes
more useful with particular BM25 parameter settings (e.g.
BM25Weight(1,0,1,0,0)) or custom weighting schemes.

Parameters:
-----------

sort_key:  value number to reorder on. Sorting is with a string
compare. If ascending is true (the default) higher is better; if
ascending is false, lower is better.

ascending:  If true, documents values which sort higher by string
compare are better. If false, the sort order is reversed. (default
true) ";

%feature("docstring")  Xapian::Enquire::set_bias "

Set the bias functor parameters.

void Xapian::Enquire::set_bias(Xapian::weight bias_weight, time_t
bias_halflife)

NB this is a temporary API for this feature.

Parameters:
-----------

bias_weight:  Maximum weight bias functor can add (and which is given
to document with a time now or in the future).

bias_halflife:  the match bias decays exponentially as you go back in
time. This sets the half-life of this decay in seconds (default 0 =>
no bias). ";

%feature("docstring")  Xapian::Enquire::get_mset "

Get (a portion of) the match set for the current query.

MSet Xapian::Enquire::get_mset(Xapian::doccount first,
Xapian::doccount maxitems, Xapian::doccount checkatleast=0, const RSet
*omrset=0, const MatchDecider *mdecider=0) const

Parameters:
-----------

first:  the first item in the result set to return. A value of zero
corresponds to the first item returned being that with the highest
score. A value of 10 corresponds to the first 10 items being ignored,
and the returned items starting at the eleventh.

maxitems:  the maximum number of items to return.

checkatleast:  the minimum number of items to check. Because the
matcher optimises, it won't consider every document which might match,
so the total number of matches is estimated. Setting checkatleast
forces it to consider that many matches and so allows for reliable
paging links.

omrset:  the relevance set to use when performing the query.

mdecider:  a decision functor to use to decide whether a given
document should be put in the MSet

A Xapian::MSet object containing the results of the query.

Parameters:
-----------

Xapian::InvalidArgumentError:  See class documentation. ";

%feature("docstring")  Xapian::Enquire::get_mset "MSet
Xapian::Enquire::get_mset(Xapian::doccount first, Xapian::doccount
maxitems, const RSet *omrset, const MatchDecider *mdecider=0) const ";

%feature("docstring")  Xapian::Enquire::get_eset "

Get the expand set for the given rset.

ESet Xapian::Enquire::get_eset(Xapian::termcount maxitems, const RSet
&omrset, int flags=0, double k=1.0, const Xapian::ExpandDecider
*edecider=0) const

Parameters:
-----------

maxitems:  the maximum number of items to return.

omrset:  the relevance set to use when performing the expand
operation.

flags:  zero or more of these values |-ed together:
Xapian::Enquire::include_query_terms query terms may be returned from
expand

Xapian::Enquire::use_exact_termfreq for multi dbs, calculate the exact
termfreq; otherwise an approximation is used which can greatly improve
efficiency, but still returns good results.

k:  the parameter k in the query expansion algorithm (default is 1.0)

edecider:  a decision functor to use to decide whether a given term
should be put in the ESet

An ESet object containing the results of the expand.

Parameters:
-----------

Xapian::InvalidArgumentError:  See class documentation. ";

%feature("docstring")  Xapian::Enquire::get_eset "

Get the expand set for the given rset.

ESet Xapian::Enquire::get_eset(Xapian::termcount maxitems, const RSet
&omrset, const Xapian::ExpandDecider *edecider) const

Parameters:
-----------

maxitems:  the maximum number of items to return.

omrset:  the relevance set to use when performing the expand
operation.

edecider:  a decision functor to use to decide whether a given term
should be put in the ESet

An ESet object containing the results of the expand.

Parameters:
-----------

Xapian::InvalidArgumentError:  See class documentation. ";

%feature("docstring")  Xapian::Enquire::get_matching_terms_begin "

Get terms which match a given document, by document id.

TermIterator Xapian::Enquire::get_matching_terms_begin(Xapian::docid
did) const

This method returns the terms in the current query which match the
given document.

It is possible for the document to have been removed from the database
between the time it is returned in an mset, and the time that this
call is made. If possible, you should specify an MSetIterator instead
of a Xapian::docid, since this will enable database backends with
suitable support to prevent this occurring.

Note that a query does not need to have been run in order to make this
call.

Parameters:
-----------

did:  The document id for which to retrieve the matching terms.

An iterator returning the terms which match the document. The terms
will be returned (as far as this makes any sense) in the same order as
the terms in the query. Terms will not occur more than once, even if
they do in the query.

Parameters:
-----------

Xapian::InvalidArgumentError:  See class documentation.

Xapian::DocNotFoundError:  The document specified could not be found
in the database. ";

%feature("docstring")  Xapian::Enquire::get_matching_terms_end "

End iterator corresponding to get_matching_terms_begin().

TermIterator Xapian::Enquire::get_matching_terms_end(Xapian::docid)
const ";

%feature("docstring")  Xapian::Enquire::get_matching_terms_begin "

Get terms which match a given document, by match set item.

TermIterator Xapian::Enquire::get_matching_terms_begin(const
MSetIterator &it) const

This method returns the terms in the current query which match the
given document.

If the underlying database has suitable support, using this call
(rather than passing a Xapian::docid) will enable the system to ensure
that the correct data is returned, and that the document has not been
deleted or changed since the query was performed.

Parameters:
-----------

it:  The iterator for which to retrieve the matching terms.

An iterator returning the terms which match the document. The terms
will be returned (as far as this makes any sense) in the same order as
the terms in the query. Terms will not occur more than once, even if
they do in the query.

Parameters:
-----------

Xapian::InvalidArgumentError:  See class documentation.

Xapian::DocNotFoundError:  The document specified could not be found
in the database. ";

%feature("docstring")  Xapian::Enquire::get_matching_terms_end "

End iterator corresponding to get_matching_terms_begin().

TermIterator Xapian::Enquire::get_matching_terms_end(const
MSetIterator &) const ";

%feature("docstring")  Xapian::Enquire::register_match_decider "

Register a MatchDecider.

void Xapian::Enquire::register_match_decider(const std::string &name,
const MatchDecider *mdecider=NULL)

Parameters:
-----------

name:  The name to register this matchdecider as.

mdecider:  The matchdecider. If omitted, then remove any matchdecider
registered with this name. ";

%feature("docstring")  Xapian::Enquire::get_description "

Introspection method.

std::string Xapian::Enquire::get_description() const

A string representing the enquire object. ";


// File: classXapian_1_1ESet.xml
%feature("docstring") Xapian::ESet "

Class representing an ordered set of expand terms (an ESet).

This set represents the results of an expand operation, which is
performed by Xapian::Enquire::get_eset(). ";

%feature("docstring")  Xapian::ESet::ESet "

Construct an empty ESet.

Xapian::ESet::ESet() ";

%feature("docstring")  Xapian::ESet::~ESet "

Destructor.

Xapian::ESet::~ESet() ";

%feature("docstring")  Xapian::ESet::ESet "

Copying is allowed (and is cheap).

Xapian::ESet::ESet(const ESet &other) ";

%feature("docstring")  Xapian::ESet::get_ebound "

A lower bound on the number of terms which are in the full set of
results of the expand.

Xapian::termcount Xapian::ESet::get_ebound() const

This will be greater than or equal to size() ";

%feature("docstring")  Xapian::ESet::size "

The number of terms in this E-Set.

Xapian::termcount Xapian::ESet::size() const ";

%feature("docstring")  Xapian::ESet::max_size "

Required to allow use as an STL container.

Xapian::termcount Xapian::ESet::max_size() const ";

%feature("docstring")  Xapian::ESet::empty "

Test if this E-Set is empty.

bool Xapian::ESet::empty() const ";

%feature("docstring")  Xapian::ESet::swap "

Swap the E-Set we point to with another.

void Xapian::ESet::swap(ESet &other) ";

%feature("docstring")  Xapian::ESet::begin "

Iterator for the terms in this E-Set.

ESetIterator Xapian::ESet::begin() const ";

%feature("docstring")  Xapian::ESet::end "

End iterator corresponding to begin().

ESetIterator Xapian::ESet::end() const ";

%feature("docstring")  Xapian::ESet::back "

Iterator pointing to the last element of this E-Set.

ESetIterator Xapian::ESet::back() const ";

%feature("docstring")  Xapian::ESet::get_description "

Introspection method.

std::string Xapian::ESet::get_description() const

A string representing this ESet. ";


// File: classXapian_1_1ESetIterator.xml
%feature("docstring") Xapian::ESetIterator "

Iterate through terms in the ESet. ";

%feature("docstring")  Xapian::ESetIterator::ESetIterator "

Create an uninitialised iterator; this cannot be used, but is
convenient syntactically.

Xapian::ESetIterator::ESetIterator() ";

%feature("docstring")  Xapian::ESetIterator::~ESetIterator "Xapian::ESetIterator::~ESetIterator() ";

%feature("docstring")  Xapian::ESetIterator::ESetIterator "

Copying is allowed (and is cheap).

Xapian::ESetIterator::ESetIterator(const ESetIterator &other) ";

%feature("docstring")  Xapian::ESetIterator::get_weight "

Get the weight of the term at the current position.

Xapian::weight Xapian::ESetIterator::get_weight() const ";

%feature("docstring")  Xapian::ESetIterator::get_description "

Returns a string describing this object.

std::string Xapian::ESetIterator::get_description() const

Introspection method. ";


// File: classXapian_1_1ExpandDecider.xml
%feature("docstring") Xapian::ExpandDecider "

Base class for expand decision functor. ";

%feature("docstring")  Xapian::ExpandDecider::~ExpandDecider "

Destructor.

virtual Xapian::ExpandDecider::~ExpandDecider() ";


// File: classXapian_1_1MatchDecider.xml
%feature("docstring") Xapian::MatchDecider "

Base class for matcher decision functor. ";

%feature("docstring")  Xapian::MatchDecider::~MatchDecider "

Destructor.

virtual Xapian::MatchDecider::~MatchDecider() ";


// File: classXapian_1_1MSet.xml
%feature("docstring") Xapian::MSet "

A match set ( MSet).

This class represents (a portion of) the results of a query. ";

%feature("docstring")  Xapian::MSet::MSet "Xapian::MSet::MSet(MSet::Internal *internal_) ";

%feature("docstring")  Xapian::MSet::MSet "

Create an empty Xapian::MSet.

Xapian::MSet::MSet() ";

%feature("docstring")  Xapian::MSet::~MSet "

Destroy a Xapian::MSet.

Xapian::MSet::~MSet() ";

%feature("docstring")  Xapian::MSet::MSet "

Copying is allowed (and is cheap).

Xapian::MSet::MSet(const MSet &other) ";

%feature("docstring")  Xapian::MSet::fetch "

Fetch the the document info for a set of items in the MSet.

void Xapian::MSet::fetch(const MSetIterator &begin, const MSetIterator
&end) const

This method causes the documents in the range specified by the
iterators to be fetched from the database, and cached in the
Xapian::MSet object. This has little effect when performing a search
across a local database, but will greatly speed up subsequent access
to the document contents when the documents are stored in a remote
database.

The iterators must be over this Xapian::MSet: undefined behaviour will
result otherwise.

Parameters:
-----------

begin:   MSetIterator for first item to fetch.

end:   MSetIterator for item after last item to fetch. ";

%feature("docstring")  Xapian::MSet::fetch "

Fetch the single item specified.

void Xapian::MSet::fetch(const MSetIterator &item) const ";

%feature("docstring")  Xapian::MSet::fetch "

Fetch all the items in the MSet.

void Xapian::MSet::fetch() const ";

%feature("docstring")  Xapian::MSet::convert_to_percent "

This converts the weight supplied to a percentage score.

Xapian::percent Xapian::MSet::convert_to_percent(Xapian::weight wt)
const

The return value will be in the range 0 to 100, and will be 0 if and
only if the item did not match the query at all. ";

%feature("docstring")  Xapian::MSet::convert_to_percent "

Return the percentage score for a particular item.

Xapian::percent Xapian::MSet::convert_to_percent(const MSetIterator
&it) const ";

%feature("docstring")  Xapian::MSet::get_termfreq "

Return the term frequency of the given query term.

Xapian::doccount Xapian::MSet::get_termfreq(const std::string &tname)
const

Parameters:
-----------

tname:  The term to look for.

Parameters:
-----------

Xapian::InvalidArgumentError:  is thrown if the term was not in the
query. ";

%feature("docstring")  Xapian::MSet::get_termweight "

Return the term weight of the given query term.

Xapian::weight Xapian::MSet::get_termweight(const std::string &tname)
const

Parameters:
-----------

tname:  The term to look for.

Parameters:
-----------

Xapian::InvalidArgumentError:  is thrown if the term was not in the
query. ";

%feature("docstring")  Xapian::MSet::get_firstitem "

The index of the first item in the result which was put into the MSet.

Xapian::doccount Xapian::MSet::get_firstitem() const

This corresponds to the parameter \"first\" specified in
Xapian::Enquire::get_mset(). A value of 0 corresponds to the highest
result being the first item in the mset. ";

%feature("docstring")  Xapian::MSet::get_matches_lower_bound "

A lower bound on the number of documents in the database which match
the query.

Xapian::doccount Xapian::MSet::get_matches_lower_bound() const

This figure takes into account collapsing of duplicates, and weighting
cutoff values.

This number is usually considerably less than the actual number of
documents which match the query. ";

%feature("docstring")  Xapian::MSet::get_matches_estimated "

An estimate for the number of documents in the database which match
the query.

Xapian::doccount Xapian::MSet::get_matches_estimated() const

This figure takes into account collapsing of duplicates, and weighting
cutoff values.

This value is returned because there is sometimes a request to display
such information. However, our experience is that presenting this
value to users causes them to worry about the large number of results,
rather than how useful those at the top of the result set are, and is
thus undesirable. ";

%feature("docstring")  Xapian::MSet::get_matches_upper_bound "

An upper bound on the number of documents in the database which match
the query.

Xapian::doccount Xapian::MSet::get_matches_upper_bound() const

This figure takes into account collapsing of duplicates, and weighting
cutoff values.

This number is usually considerably greater than the actual number of
documents which match the query. ";

%feature("docstring")  Xapian::MSet::get_max_possible "

The maximum possible weight in the mset.

Xapian::weight Xapian::MSet::get_max_possible() const

This weight is likely not to be attained in the set of results, but
represents an upper bound on the weight which a document could attain
for the given query. ";

%feature("docstring")  Xapian::MSet::get_max_attained "

The greatest weight which is attained by any document in the database.

Xapian::weight Xapian::MSet::get_max_attained() const

If firstitem == 0, this is the weight of the first entry in items.

If no documents are found by the query, this will be 0.

Note that calculation of max_attained requires calculation of at least
one result item - therefore, if no items were requested when the query
was performed (by specifying maxitems = 0 in
Xapian::Enquire::get_mset()), this value will be 0. ";

%feature("docstring")  Xapian::MSet::size "

The number of items in this MSet.

Xapian::doccount Xapian::MSet::size() const ";

%feature("docstring")  Xapian::MSet::max_size "

Required to allow use as an STL container.

Xapian::doccount Xapian::MSet::max_size() const ";

%feature("docstring")  Xapian::MSet::empty "

Test if this MSet is empty.

bool Xapian::MSet::empty() const ";

%feature("docstring")  Xapian::MSet::swap "

Swap the MSet we point to with another.

void Xapian::MSet::swap(MSet &other) ";

%feature("docstring")  Xapian::MSet::begin "

Iterator for the terms in this MSet.

MSetIterator Xapian::MSet::begin() const ";

%feature("docstring")  Xapian::MSet::end "

End iterator corresponding to begin().

MSetIterator Xapian::MSet::end() const ";

%feature("docstring")  Xapian::MSet::back "

Iterator pointing to the last element of this MSet.

MSetIterator Xapian::MSet::back() const ";

%feature("docstring")  Xapian::MSet::get_description "

Returns a string representing the mset.

std::string Xapian::MSet::get_description() const

Introspection method. ";


// File: classXapian_1_1MSetIterator.xml
%feature("docstring") Xapian::MSetIterator "

An iterator pointing to items in an MSet.

This is used for access to individual results of a match. ";

%feature("docstring")  Xapian::MSetIterator::MSetIterator "

Create an uninitialised iterator; this cannot be used, but is
convenient syntactically.

Xapian::MSetIterator::MSetIterator() ";

%feature("docstring")  Xapian::MSetIterator::~MSetIterator "Xapian::MSetIterator::~MSetIterator() ";

%feature("docstring")  Xapian::MSetIterator::MSetIterator "

Copying is allowed (and is cheap).

Xapian::MSetIterator::MSetIterator(const MSetIterator &other) ";

%feature("docstring")  Xapian::MSetIterator::get_document "

Get a Xapian::Document object for the current position.

Xapian::Document Xapian::MSetIterator::get_document() const

This method returns a Xapian::Document object which provides the
information about the document pointed to by the MSetIterator.

If the underlying database has suitable support, using this call
(rather than asking the database for a document based on its document
ID) will enable the system to ensure that the correct data is
returned, and that the document has not been deleted or changed since
the query was performed.

A Xapian::Document object containing the document data.

Parameters:
-----------

Xapian::DocNotFoundError:  The document specified could not be found
in the database. ";

%feature("docstring")  Xapian::MSetIterator::get_rank "

Get the rank of the document at the current position.

Xapian::doccount Xapian::MSetIterator::get_rank() const

The rank is the position that this document is at in the ordered list
of results of the query. The document judged \"most relevant\" will
have rank of 0. ";

%feature("docstring")  Xapian::MSetIterator::get_weight "

Get the weight of the document at the current position.

Xapian::weight Xapian::MSetIterator::get_weight() const ";

%feature("docstring")  Xapian::MSetIterator::get_collapse_key "

Get the collapse key for this document.

std::string Xapian::MSetIterator::get_collapse_key() const ";

%feature("docstring")  Xapian::MSetIterator::get_collapse_count "

Get an estimate of the number of documents that have been collapsed
into this one.

Xapian::doccount Xapian::MSetIterator::get_collapse_count() const

The estimate will always be less than or equal to the actual number of
other documents satisfying the match criteria with the same collapse
key as this document.

This method may return 0 even though there are other documents with
the same collapse key which satisfying the match criteria. However if
this method returns non-zero, there definitely are other such
documents. So this method may be used to inform the user that there
are \"at least N other matches in this group\", or to control whether
to offer a \"show other documents in this group\" feature (but note
that it may not offer it in every case where it would show other
documents). ";

%feature("docstring")  Xapian::MSetIterator::get_percent "

This returns the weight of the document as a percentage score.

Xapian::percent Xapian::MSetIterator::get_percent() const

The return value will be in the range 0 to 100: 0 meaning that the
item did not match the query at all. ";

%feature("docstring")  Xapian::MSetIterator::get_description "

Returns a string describing this object.

std::string Xapian::MSetIterator::get_description() const

Introspection method. ";


// File: classXapian_1_1Internal_1_1RefCntBase.xml
%feature("docstring") Xapian::Internal::RefCntBase "

Reference counted internal classes should inherit from RefCntBase.

This gives the object a reference count used by RefCntPtr. ";

%feature("docstring")  Xapian::Internal::RefCntBase::RefCntBase "

The constructor, which initialises the ref_count to 0.

Xapian::Internal::RefCntBase::RefCntBase() ";


// File: classXapian_1_1Internal_1_1RefCntPtr.xml
%feature("docstring") Xapian::Internal::RefCntPtr "

A reference-counted pointer.

Can be used with any class derived from RefCntBase, as long as it is
allocated on the heap by new (not new[]!). ";

%feature("docstring")  Xapian::Internal::RefCntPtr::get "T *
Xapian::Internal::RefCntPtr< T >::get() const ";

%feature("docstring")  Xapian::Internal::RefCntPtr::RefCntPtr "

Make a RefCntPtr for an object which may already have reference
counted pointers.

Xapian::Internal::RefCntPtr< T >::RefCntPtr(T *dest_)

You usually pass in a newly created object, or an object may pass in
\"this\" to get a RefCntPtr to itself to pass to other classes. (e.g.
a database might pass a newly created postlist a reference counted
pointer to itself.) ";

%feature("docstring")  Xapian::Internal::RefCntPtr::RefCntPtr "Xapian::Internal::RefCntPtr< T >::RefCntPtr() ";

%feature("docstring")  Xapian::Internal::RefCntPtr::RefCntPtr "Xapian::Internal::RefCntPtr< T >::RefCntPtr(const RefCntPtr &other) ";

%feature("docstring")  Xapian::Internal::RefCntPtr::~RefCntPtr "Xapian::Internal::RefCntPtr< T >::~RefCntPtr() ";

%feature("docstring")  Xapian::Internal::RefCntPtr::RefCntPtr "Xapian::Internal::RefCntPtr< T >::RefCntPtr(const RefCntPtr< U >
&other) ";


// File: classXapian_1_1RSet.xml
%feature("docstring") Xapian::RSet "

A relevance set (R-Set).

This is the set of documents which are marked as relevant, for use in
modifying the term weights, and in performing query expansion. ";

%feature("docstring")  Xapian::RSet::RSet "

Copy constructor.

Xapian::RSet::RSet(const RSet &rset) ";

%feature("docstring")  Xapian::RSet::RSet "

Default constructor.

Xapian::RSet::RSet() ";

%feature("docstring")  Xapian::RSet::~RSet "

Destructor.

Xapian::RSet::~RSet() ";

%feature("docstring")  Xapian::RSet::size "

The number of documents in this R-Set.

Xapian::doccount Xapian::RSet::size() const ";

%feature("docstring")  Xapian::RSet::empty "

Test if this R-Set is empty.

bool Xapian::RSet::empty() const ";

%feature("docstring")  Xapian::RSet::add_document "

Add a document to the relevance set.

void Xapian::RSet::add_document(Xapian::docid did) ";

%feature("docstring")  Xapian::RSet::add_document "

Add a document to the relevance set.

void Xapian::RSet::add_document(const Xapian::MSetIterator &i) ";

%feature("docstring")  Xapian::RSet::remove_document "

Remove a document from the relevance set.

void Xapian::RSet::remove_document(Xapian::docid did) ";

%feature("docstring")  Xapian::RSet::remove_document "

Remove a document from the relevance set.

void Xapian::RSet::remove_document(const Xapian::MSetIterator &i) ";

%feature("docstring")  Xapian::RSet::contains "

Test if a given document in the relevance set.

bool Xapian::RSet::contains(Xapian::docid did) const ";

%feature("docstring")  Xapian::RSet::contains "

Test if a given document in the relevance set.

bool Xapian::RSet::contains(const Xapian::MSetIterator &i) const ";

%feature("docstring")  Xapian::RSet::get_description "

Introspection method.

std::string Xapian::RSet::get_description() const

A string representing this RSet. ";


// File: classXapian_1_1TradWeight.xml
%feature("docstring") Xapian::TradWeight "

Traditional probabilistic weighting scheme.

This class implements the Traditional Probabilistic Weighting scheme,
as described by the early papers on Probabilistic Retrieval. BM25
generally gives better results.

The Traditional weighting scheme formula is \\\\[
\\\\sum_{t}\\\\frac{f_{t,d}}{k.L_{d}+f_{t,d}}.w_{t} \\\\] where
$w_{t}$ is the termweight of term t

$f_{t,d}$ is the within document frequency of term t in document d

$L_{d}$ is the normalised length of document d

$k$ is a user specifiable parameter

TradWeight(k) is equivalent to BM25Weight(k, 0, 0, 1, 0), except that
the latter returns weights (k+1) times larger. ";

%feature("docstring")  Xapian::TradWeight::TradWeight "

Construct a TradWeight.

Xapian::TradWeight::TradWeight(double k)

Parameters:
-----------

k:  parameter governing the importance of within document frequency
and document length - any non-negative number (0 meaning to ignore wdf
and doc length when calculating weights). Default is 1. ";

%feature("docstring")  Xapian::TradWeight::TradWeight "Xapian::TradWeight::TradWeight() ";

%feature("docstring")  Xapian::TradWeight::clone "

Return a new weight object of this type.

TradWeight* Xapian::TradWeight::clone() const

A subclass called FooWeight taking parameters param1 and param2 should
implement this as:

virtual FooWeight * clone() const { return new FooWeight(param1,
param2); } ";

%feature("docstring")  Xapian::TradWeight::~TradWeight "Xapian::TradWeight::~TradWeight() ";

%feature("docstring")  Xapian::TradWeight::name "

Name of the weighting scheme.

std::string Xapian::TradWeight::name() const

If the subclass is called FooWeight, this should return \"Foo\". ";

%feature("docstring")  Xapian::TradWeight::serialise "

Serialise object parameters into a string.

std::string Xapian::TradWeight::serialise() const ";

%feature("docstring")  Xapian::TradWeight::unserialise "

Create object given string serialisation returned by serialise().

TradWeight* Xapian::TradWeight::unserialise(const std::string &s)
const ";

%feature("docstring")  Xapian::TradWeight::get_sumpart "

Get a weight which is part of the sum over terms being performed.

Xapian::weight Xapian::TradWeight::get_sumpart(Xapian::termcount wdf,
Xapian::doclength len) const

This returns a weight for a given term and document. These weights are
summed to give a total weight for the document.

Parameters:
-----------

wdf:  the within document frequency of the term.

len:  the (unnormalised) document length. ";

%feature("docstring")  Xapian::TradWeight::get_maxpart "

Gets the maximum value that get_sumpart() may return.

Xapian::weight Xapian::TradWeight::get_maxpart() const

This is used in optimising searches, by having the postlist tree decay
appropriately when parts of it can have limited, or no, further
effect. ";

%feature("docstring")  Xapian::TradWeight::get_sumextra "

Get an extra weight for a document to add to the sum calculated over
the query terms.

Xapian::weight Xapian::TradWeight::get_sumextra(Xapian::doclength len)
const

This returns a weight for a given document, and is used by some
weighting schemes to account for influence such as document length.

Parameters:
-----------

len:  the (unnormalised) document length. ";

%feature("docstring")  Xapian::TradWeight::get_maxextra "

Gets the maximum value that get_sumextra() may return.

Xapian::weight Xapian::TradWeight::get_maxextra() const

This is used in optimising searches. ";

%feature("docstring")  Xapian::TradWeight::get_sumpart_needs_doclength
"

return false if the weight object doesn't need doclength

bool Xapian::TradWeight::get_sumpart_needs_doclength() const ";


// File: classXapian_1_1Weight.xml
%feature("docstring") Xapian::Weight "

Abstract base class for weighting schemes. ";

%feature("docstring")  Xapian::Weight::Weight "Xapian::Weight::Weight() ";

%feature("docstring")  Xapian::Weight::~Weight "virtual
Xapian::Weight::~Weight() ";

%feature("docstring")  Xapian::Weight::create "

Create a new weight object of the same type as this and initialise it
with the specified statistics.

Weight* Xapian::Weight::create(const Internal *internal_,
Xapian::doclength querysize_, Xapian::termcount wqf_, const
std::string &tname_) const

You shouldn't call this method yourself - it's called by Enquire.

Parameters:
-----------

internal_:  Object to ask for collection statistics.

querysize_:   Query size.

wqf_:  Within query frequency of term this object is associated with.

tname_:  Term which this object is associated with. ";

%feature("docstring")  Xapian::Weight::name "

Name of the weighting scheme.

virtual std::string Xapian::Weight::name() const=0

If the subclass is called FooWeight, this should return \"Foo\". ";

%feature("docstring")  Xapian::Weight::serialise "

Serialise object parameters into a string.

virtual std::string Xapian::Weight::serialise() const=0 ";

%feature("docstring")  Xapian::Weight::unserialise "

Create object given string serialisation returned by serialise().

virtual Weight* Xapian::Weight::unserialise(const std::string &s)
const =0 ";

%feature("docstring")  Xapian::Weight::get_sumpart "

Get a weight which is part of the sum over terms being performed.

virtual Xapian::weight Xapian::Weight::get_sumpart(Xapian::termcount
wdf, Xapian::doclength len) const =0

This returns a weight for a given term and document. These weights are
summed to give a total weight for the document.

Parameters:
-----------

wdf:  the within document frequency of the term.

len:  the (unnormalised) document length. ";

%feature("docstring")  Xapian::Weight::get_maxpart "

Gets the maximum value that get_sumpart() may return.

virtual Xapian::weight Xapian::Weight::get_maxpart() const=0

This is used in optimising searches, by having the postlist tree decay
appropriately when parts of it can have limited, or no, further
effect. ";

%feature("docstring")  Xapian::Weight::get_sumextra "

Get an extra weight for a document to add to the sum calculated over
the query terms.

virtual Xapian::weight Xapian::Weight::get_sumextra(Xapian::doclength
len) const=0

This returns a weight for a given document, and is used by some
weighting schemes to account for influence such as document length.

Parameters:
-----------

len:  the (unnormalised) document length. ";

%feature("docstring")  Xapian::Weight::get_maxextra "

Gets the maximum value that get_sumextra() may return.

virtual Xapian::weight Xapian::Weight::get_maxextra() const=0

This is used in optimising searches. ";

%feature("docstring")  Xapian::Weight::get_sumpart_needs_doclength "

return false if the weight object doesn't need doclength

virtual bool Xapian::Weight::get_sumpart_needs_doclength() const ";


// File: classXapian_1_1WritableDatabase.xml
%feature("docstring") Xapian::WritableDatabase "

This class provides read/write access to a database. ";

%feature("docstring")  Xapian::WritableDatabase::~WritableDatabase "

Destroy this handle on the database.

virtual Xapian::WritableDatabase::~WritableDatabase()

If there are no copies of this object remaining, the database will be
closed. If there are any transactions in progress these will be
aborted as if cancel_transaction had been called. ";

%feature("docstring")  Xapian::WritableDatabase::WritableDatabase "

Create an empty WritableDatabase.

Xapian::WritableDatabase::WritableDatabase() ";

%feature("docstring")  Xapian::WritableDatabase::WritableDatabase "

Open a database for update, automatically determining the database
backend to use.

Xapian::WritableDatabase::WritableDatabase(const std::string &path,
int action)

If the database is to be created, Xapian will try to create the
directory indicated by path if it doesn't already exist (but only the
leaf directory, not recursively).

Parameters:
-----------

path:  directory that the database is stored in.

action:  one of:  Xapian::DB_CREATE_OR_OPEN open for read/write;
create if no db exists

Xapian::DB_CREATE create new database; fail if db exists

Xapian::DB_CREATE_OR_OVERWRITE overwrite existing db; create if none
exists

Xapian::DB_OPEN open for read/write; fail if no db exists ";

%feature("docstring")  Xapian::WritableDatabase::WritableDatabase "Xapian::WritableDatabase::WritableDatabase(Database::Internal
*internal) ";

%feature("docstring")  Xapian::WritableDatabase::WritableDatabase "

Copying is allowed.

Xapian::WritableDatabase::WritableDatabase(const WritableDatabase
&other)

The internals are reference counted, so copying is cheap. ";

%feature("docstring")  Xapian::WritableDatabase::flush "

Flush to disk any modifications made to the database.

void Xapian::WritableDatabase::flush()

For efficiency reasons, when performing multiple updates to a database
it is best (indeed, almost essential) to make as many modifications as
memory will permit in a single pass through the database. To ensure
this, Xapian batches up modifications.

Flush may be called at any time to ensure that the modifications which
have been made are written to disk: if the flush succeeds, all the
preceding modifications will have been written to disk.

If any of the modifications fail, an exception will be thrown and the
database will be left in a state in which each separate addition,
replacement or deletion operation has either been fully performed or
not performed at all: it is then up to the application to work out
which operations need to be repeated.

It's not valid to call flush within a transaction.

Beware of calling flush too frequently: this will have a severe
performance cost.

Note that flush need not be called explicitly: it will be called
automatically when the database is closed, or when a sufficient number
of modifications have been made.

Parameters:
-----------

Xapian::DatabaseError:  will be thrown if a problem occurs while
modifying the database.

Xapian::DatabaseCorruptError:  will be thrown if the database is in a
corrupt state.

Xapian::DatabaseLockError:  will be thrown if a lock couldn't be
acquired on the database. ";

%feature("docstring")  Xapian::WritableDatabase::begin_transaction "

Begin a transaction.

void Xapian::WritableDatabase::begin_transaction(bool flushed=true)

In Xapian a transaction is a group of modifications to the database
which are linked such that either all will be applied simultaneously
or none will be applied at all. Even in the case of a power failure,
this characteristic should be preserved (as long as the filesystem
isn't corrupted, etc).

A transaction is started with begin_transaction() and can either be
committed by calling commit_transaction() or aborted by calling
cancel_transaction().

By default, a transaction implicitly calls flush before and after so
that the modifications stand and fall without affecting modifications
before or after.

The downside of this flushing is that small transactions cause
modifications to be frequently flushed which can harm indexing
performance in the same way that explicitly calling flush frequently
can.

If you're applying atomic groups of changes and only wish to ensure
that each group is either applied or not applied, then you can prevent
the automatic flush before and after the transaction by starting the
transaction with begin_transaction(false). However, if
cancel_transaction is called (or if commit_transaction isn't called
before the WritableDatabase object is destroyed) then any changes
which were pending before the transaction began will also be
discarded.

Transactions aren't currently supported by the InMemory backend.

Parameters:
-----------

Xapian::UnimplementedError:  will be thrown if transactions are not
available for this database type.

Xapian::InvalidOperationError:  will be thrown if this is called at an
invalid time, such as when a transaction is already in progress. ";

%feature("docstring")  Xapian::WritableDatabase::commit_transaction "

Complete the transaction currently in progress.

void Xapian::WritableDatabase::commit_transaction()

If this method completes successfully and this is a flushed
transaction, all the database modifications made during the
transaction will have been committed to the database.

If an error occurs, an exception will be thrown, and none of the
modifications made to the database during the transaction will have
been applied to the database.

In all cases the transaction will no longer be in progress.

Parameters:
-----------

Xapian::DatabaseError:  will be thrown if a problem occurs while
modifying the database.

Xapian::DatabaseCorruptError:  will be thrown if the database is in a
corrupt state.

Xapian::InvalidOperationError:  will be thrown if a transaction is not
currently in progress.

Xapian::UnimplementedError:  will be thrown if transactions are not
available for this database type. ";

%feature("docstring")  Xapian::WritableDatabase::cancel_transaction "

Abort the transaction currently in progress, discarding the potential
modifications made to the database.

void Xapian::WritableDatabase::cancel_transaction()

If an error occurs in this method, an exception will be thrown, but
the transaction will be cancelled anyway.

Parameters:
-----------

Xapian::DatabaseError:  will be thrown if a problem occurs while
modifying the database.

Xapian::DatabaseCorruptError:  will be thrown if the database is in a
corrupt state.

Xapian::InvalidOperationError:  will be thrown if a transaction is not
currently in progress.

Xapian::UnimplementedError:  will be thrown if transactions are not
available for this database type. ";

%feature("docstring")  Xapian::WritableDatabase::add_document "

Add a new document to the database.

Xapian::docid Xapian::WritableDatabase::add_document(const
Xapian::Document &document)

This method adds the specified document to the database, returning a
newly allocated document ID. Automatically allocated document IDs come
from a per-database monotonically increasing counter, so IDs from
deleted documents won't be reused.

If you want to specify the document ID to be used, you should call
replace_document() instead.

Note that changes to the database won't be immediately committed to
disk; see flush() for more details.

As with all database modification operations, the effect is atomic:
the document will either be fully added, or the document fails to be
added and an exception is thrown (possibly at a later time when flush
is called or the database is closed).

Parameters:
-----------

document:  The new document to be added.

The document ID of the newly added document.

Parameters:
-----------

Xapian::DatabaseError:  will be thrown if a problem occurs while
writing to the database.

Xapian::DatabaseCorruptError:  will be thrown if the database is in a
corrupt state. ";

%feature("docstring")  Xapian::WritableDatabase::delete_document "

Delete a document from the database.

void Xapian::WritableDatabase::delete_document(Xapian::docid did)

This method removes the document with the specified document ID from
the database.

Note that changes to the database won't be immediately committed to
disk; see flush() for more details.

As with all database modification operations, the effect is atomic:
the document will either be fully removed, or the document fails to be
removed and an exception is thrown (possibly at a later time when
flush is called or the database is closed).

Parameters:
-----------

did:  The document ID of the document to be removed.

Parameters:
-----------

Xapian::DatabaseError:  will be thrown if a problem occurs while
writing to the database.

Xapian::DatabaseCorruptError:  will be thrown if the database is in a
corrupt state. ";

%feature("docstring")  Xapian::WritableDatabase::delete_document "

Delete any documents indexed by a term from the database.

void Xapian::WritableDatabase::delete_document(const std::string
&unique_term)

This method removes any documents indexed by the specified term from
the database.

The intended use is to allow UIDs from another system to easily be
mapped to terms in Xapian, although this method probably has other
uses.

Parameters:
-----------

unique_term:  The term to remove references to.

Parameters:
-----------

Xapian::DatabaseError:  will be thrown if a problem occurs while
writing to the database.

Xapian::DatabaseCorruptError:  will be thrown if the database is in a
corrupt state. ";

%feature("docstring")  Xapian::WritableDatabase::replace_document "

Replace a given document in the database.

void Xapian::WritableDatabase::replace_document(Xapian::docid did,
const Xapian::Document &document)

This method replaces the document with the specified document ID. If
document ID did isn't currently used, the document will be added with
document ID did.

Note that changes to the database won't be immediately committed to
disk; see flush() for more details.

As with all database modification operations, the effect is atomic:
the document will either be fully replaced, or the document fails to
be replaced and an exception is thrown (possibly at a later time when
flush is called or the database is closed).

Parameters:
-----------

did:  The document ID of the document to be replaced.

document:  The new document.

Parameters:
-----------

Xapian::DatabaseError:  will be thrown if a problem occurs while
writing to the database.

Xapian::DatabaseCorruptError:  will be thrown if the database is in a
corrupt state. ";

%feature("docstring")  Xapian::WritableDatabase::replace_document "

Replace any documents matching a term.

Xapian::docid Xapian::WritableDatabase::replace_document(const
std::string &unique_term, const Xapian::Document &document)

This method replaces any documents indexed by the specified term with
the specified document. If any documents are indexed by the term, the
lowest document ID will be used for the document, otherwise a new
document ID will be generated as for add_document.

The intended use is to allow UIDs from another system to easily be
mapped to terms in Xapian, although this method probably has other
uses.

Note that changes to the database won't be immediately committed to
disk; see flush() for more details.

As with all database modification operations, the effect is atomic:
the document(s) will either be fully replaced, or the document(s) fail
to be replaced and an exception is thrown (possibly at a later time
when flush is called or the database is closed).

Parameters:
-----------

unique_term:  The \"unique\" term.

document:  The new document.

The document ID that document was given.

Parameters:
-----------

Xapian::DatabaseError:  will be thrown if a problem occurs while
writing to the database.

Xapian::DatabaseCorruptError:  will be thrown if the database is in a
corrupt state. ";

%feature("docstring")  Xapian::WritableDatabase::get_description "

Introspection method.

std::string Xapian::WritableDatabase::get_description() const

A string describing this object. ";


// File: classXapian.xml
%feature("docstring") Xapian "

Decide if a Xapian::Error exception should be ignored.

You can create your own subclass of this class and pass in an instance
of it when you construct a Xapian::Enquire object. Xapian::Error
exceptions which happen during the match process are passed to this
object and it can decide whether they should propagate or whether
Enquire should attempt to continue.

The motivation is to allow searching over remote databases to handle a
remote server which has died (both to allow results to be returned,
and also so that such errors can be logged and dead servers
temporarily removed from use). ";

%feature("docstring")  Xapian::ErrorHandler "

Default constructor.

Xapian::ErrorHandler() ";

%feature("docstring")  Xapian::~ErrorHandler "

We require a virtual destructor because we have virtual methods.

virtual Xapian::~ErrorHandler() ";


// File: classXapian_1_1DateValueRangeProcessor.xml
%feature("docstring") Xapian::DateValueRangeProcessor "";

%feature("docstring")
Xapian::DateValueRangeProcessor::DateValueRangeProcessor "Xapian::Dat
eValueRangeProcessor::DateValueRangeProcessor(Xapian::valueno valno_,
bool prefer_mdy_=false, int epoch_year_=1970) ";


// File: classXapian_1_1DocIDWrapper.xml
%feature("docstring") Xapian::DocIDWrapper "

A wrapper class for a docid which returns the docid if dereferenced
with *.

We need this to implement input_iterator semantics. ";

%feature("docstring")  Xapian::DocIDWrapper::DocIDWrapper "Xapian::DocIDWrapper::DocIDWrapper(docid did_) ";


// File: classXapian_1_1ExpandDeciderAnd.xml
%feature("docstring") Xapian::ExpandDeciderAnd "

An expand decision functor which can be used to join two functors with
an AND operation. ";

%feature("docstring")  Xapian::ExpandDeciderAnd::ExpandDeciderAnd "

Constructor, which takes as arguments the two decision functors to AND
together.

Xapian::ExpandDeciderAnd::ExpandDeciderAnd(const ExpandDecider *left_,
const ExpandDecider *right_)

ExpandDeciderAnd will not delete its sub-functors. ";


// File: classXapian_1_1ExpandDeciderFilterTerms.xml
%feature("docstring") Xapian::ExpandDeciderFilterTerms "

One useful expand decision functor, which provides a way of filtering
out a fixed list of terms from the expand set. ";

%feature("docstring")
Xapian::ExpandDeciderFilterTerms::ExpandDeciderFilterTerms "

Constructor, which takes a list of terms which will be filtered out.

Xapian::ExpandDeciderFilterTerms::ExpandDeciderFilterTerms(Xapian::Ter
mIterator terms, Xapian::TermIterator termsend) ";


// File: classXapian_1_1NumberValueRangeProcessor.xml
%feature("docstring") Xapian::NumberValueRangeProcessor "";

%feature("docstring")
Xapian::NumberValueRangeProcessor::NumberValueRangeProcessor "Xapian:
:NumberValueRangeProcessor::NumberValueRangeProcessor(Xapian::valueno
valno_) ";

%feature("docstring")
Xapian::NumberValueRangeProcessor::NumberValueRangeProcessor "Xapian:
:NumberValueRangeProcessor::NumberValueRangeProcessor(Xapian::valueno
valno_, const std::string &str_, bool prefix_=true) ";


// File: classXapian_1_1PositionIterator.xml
%feature("docstring") Xapian::PositionIterator "

An iterator pointing to items in a list of positions. ";

%feature("docstring")  Xapian::PositionIterator::PositionIterator "Xapian::PositionIterator::PositionIterator(Internal *internal_) ";

%feature("docstring")  Xapian::PositionIterator::PositionIterator "

Default constructor - for declaring an uninitialised iterator.

Xapian::PositionIterator::PositionIterator() ";

%feature("docstring")  Xapian::PositionIterator::~PositionIterator "

Destructor.

Xapian::PositionIterator::~PositionIterator() ";

%feature("docstring")  Xapian::PositionIterator::PositionIterator "

Copying is allowed.

Xapian::PositionIterator::PositionIterator(const PositionIterator &o)

The internals are reference counted, so copying is also cheap. ";

%feature("docstring")  Xapian::PositionIterator::skip_to "void
Xapian::PositionIterator::skip_to(Xapian::termpos pos) ";

%feature("docstring")  Xapian::PositionIterator::get_description "

Returns a string describing this object.

std::string Xapian::PositionIterator::get_description() const

Introspection method. ";


// File: classXapian_1_1PostingIterator.xml
%feature("docstring") Xapian::PostingIterator "

An iterator pointing to items in a list of postings. ";

%feature("docstring")  Xapian::PostingIterator::PostingIterator "

Default constructor - for declaring an uninitialised iterator.

Xapian::PostingIterator::PostingIterator() ";

%feature("docstring")  Xapian::PostingIterator::~PostingIterator "

Destructor.

Xapian::PostingIterator::~PostingIterator() ";

%feature("docstring")  Xapian::PostingIterator::PostingIterator "

Copying is allowed.

Xapian::PostingIterator::PostingIterator(const PostingIterator &other)

The internals are reference counted, so copying is also cheap. ";

%feature("docstring")  Xapian::PostingIterator::skip_to "

Skip the iterator to document did, or the first document after did if
did isn't in the list of documents being iterated.

void Xapian::PostingIterator::skip_to(Xapian::docid did) ";

%feature("docstring")  Xapian::PostingIterator::get_doclength "

Get the length of the document at the current position in the
postlist.

Xapian::doclength Xapian::PostingIterator::get_doclength() const

This information may be stored in the postlist, in which case this
lookup should be extremely fast (indeed, not require further disk
access). If the information is not present in the postlist, it will be
retrieved from the database, at a greater performance cost. ";

%feature("docstring")  Xapian::PostingIterator::get_wdf "

Get the within document frequency of the document at the current
position in the postlist.

Xapian::termcount Xapian::PostingIterator::get_wdf() const ";

%feature("docstring")  Xapian::PostingIterator::positionlist_begin "

Return PositionIterator pointing to start of positionlist for current
document.

PositionIterator Xapian::PostingIterator::positionlist_begin() const
";

%feature("docstring")  Xapian::PostingIterator::positionlist_end "

Return PositionIterator pointing to end of positionlist for current
document.

PositionIterator Xapian::PostingIterator::positionlist_end() const ";

%feature("docstring")  Xapian::PostingIterator::get_description "

Returns a string describing this object.

std::string Xapian::PostingIterator::get_description() const

Introspection method. ";


// File: classXapian_1_1Query.xml
%feature("docstring") Xapian::Query "

Class representing a query.

Queries are represented as a tree of objects. ";

%feature("docstring")  Xapian::Query::Query "

Copy constructor.

Xapian::Query::Query(const Query &copyme) ";

%feature("docstring")  Xapian::Query::Query "

Default constructor: makes an empty query which matches no documents.

Xapian::Query::Query()

Also useful for defining a Query object to be assigned to later.

An exception will be thrown if an attempt is made to use an undefined
query when building up a composite query. ";

%feature("docstring")  Xapian::Query::~Query "

Destructor.

Xapian::Query::~Query() ";

%feature("docstring")  Xapian::Query::Query "

A query consisting of a single term.

Xapian::Query::Query(const std::string &tname_, Xapian::termcount
wqf_=1, Xapian::termpos pos_=0) ";

%feature("docstring")  Xapian::Query::Query "

A query consisting of two subqueries, opp-ed together.

Xapian::Query::Query(Query::op op_, const Query &left, const Query
&right) ";

%feature("docstring")  Xapian::Query::Query "

A query consisting of two termnames opp-ed together.

Xapian::Query::Query(Query::op op_, const std::string &left, const
std::string &right) ";

%feature("docstring")  Xapian::Query::Query "

Combine a number of Xapian::Query-s with the specified operator.

Xapian::Query::Query(Query::op op_, Iterator qbegin, Iterator qend,
Xapian::termcount parameter=0)

The Xapian::Query objects are specified with begin and end iterators.

AND, OR, NEAR and PHRASE can take any number of subqueries. Other
operators take exactly two subqueries.

The iterators may be to Xapian::Query objects, pointers to
Xapian::Query objects, or termnames (std::string-s).

For NEAR and PHRASE, a window size can be specified in parameter.

For ELITE_SET, the elite set size can be specified in parameter. ";

%feature("docstring")  Xapian::Query::Query "

Apply the specified operator to a single Xapian::Query object.

Xapian::Query::Query(Query::op op_, Xapian::Query q) ";

%feature("docstring")  Xapian::Query::Query "

Construct a range query on a document value.

Xapian::Query::Query(Query::op op_, Xapian::valueno valno, const
std::string &begin, const std::string &end) ";

%feature("docstring")  Xapian::Query::get_length "

Get the length of the query, used by some ranking formulae.

Xapian::termcount Xapian::Query::get_length() const

This value is calculated automatically - if you want to override it
you can pass a different value to Enquire::set_query(). ";

%feature("docstring")  Xapian::Query::get_terms_begin "

Return a Xapian::TermIterator returning all the terms in the query, in
order of termpos.

TermIterator Xapian::Query::get_terms_begin() const

If multiple terms have the same term position, their order is
unspecified. Duplicates (same term and termpos) will be removed. ";

%feature("docstring")  Xapian::Query::get_terms_end "

Return a Xapian::TermIterator to the end of the list of terms in the
query.

TermIterator Xapian::Query::get_terms_end() const ";

%feature("docstring")  Xapian::Query::empty "

Test if the query is empty (i.e.

bool Xapian::Query::empty() const

was constructed using the default ctor or with an empty iterator
ctor). ";

%feature("docstring")  Xapian::Query::XAPIAN_DEPRECATED "Xapian::Query::XAPIAN_DEPRECATED(bool is_empty() const)

Deprecated Deprecated alias for empty() ";

%feature("docstring")  Xapian::Query::get_description "

Returns a string representing the query.

std::string Xapian::Query::get_description() const

Introspection method. ";

%feature("docstring")  Xapian::Query::Internal "

Copy constructor.

Xapian::Query::Internal(const Query::Internal &copyme) ";

%feature("docstring")  Xapian::Query::Internal "

A query consisting of a single term.

Xapian::Query::Internal(const std::string &tname_, Xapian::termcount
wqf_=1, Xapian::termpos term_pos_=0) ";

%feature("docstring")  Xapian::Query::Internal "

Create internals given only the operator and a parameter.

Xapian::Query::Internal(op_t op_, Xapian::termcount parameter) ";

%feature("docstring")  Xapian::Query::Internal "

Construct a range query on a document value.

Xapian::Query::Internal(op_t op_, Xapian::valueno valno, const
std::string &begin, const std::string &end) ";

%feature("docstring")  Xapian::Query::~Internal "

Destructor.

Xapian::Query::~Internal() ";

%feature("docstring")  Xapian::Query::add_subquery "

Add a subquery.

void Xapian::Query::add_subquery(const Query::Internal *subq) ";

%feature("docstring")  Xapian::Query::end_construction "

Finish off the construction.

Query::Internal* Xapian::Query::end_construction() ";

%feature("docstring")  Xapian::Query::serialise "

Return a string in an easily parsed form which contains all the
information in a query.

std::string Xapian::Query::serialise() const ";

%feature("docstring")  Xapian::Query::get_description "

Returns a string representing the query.

std::string Xapian::Query::get_description() const

Introspection method. ";

%feature("docstring")  Xapian::Query::get_length "

Get the length of the query, used by some ranking formulae.

Xapian::termcount Xapian::Query::get_length() const

This value is calculated automatically - if you want to override it
you can pass a different value to Enquire::set_query(). ";

%feature("docstring")  Xapian::Query::get_terms "

Return an iterator over all the terms in the query, in order of
termpos.

TermIterator Xapian::Query::get_terms() const

If multiple terms have the same term position, their order is
unspecified. Duplicates (same term and termpos) will be removed. ";

%feature("docstring")  Xapian::Query::Query "Xapian::Query::Query(Query::op op_, Iterator qbegin, Iterator qend,
termcount parameter) ";


// File: classXapian_1_1QueryParser.xml
%feature("docstring") Xapian::QueryParser "

Build a Xapian::Query object from a user query string. ";

%feature("docstring")  Xapian::QueryParser::QueryParser "

Copy constructor.

Xapian::QueryParser::QueryParser(const QueryParser &o) ";

%feature("docstring")  Xapian::QueryParser::QueryParser "

Default constructor.

Xapian::QueryParser::QueryParser() ";

%feature("docstring")  Xapian::QueryParser::~QueryParser "

Destructor.

Xapian::QueryParser::~QueryParser() ";

%feature("docstring")  Xapian::QueryParser::set_stemmer "

Set the stemmer.

void Xapian::QueryParser::set_stemmer(const Xapian::Stem &stemmer) ";

%feature("docstring")  Xapian::QueryParser::set_stemming_strategy "

Set the stemming strategy.

void Xapian::QueryParser::set_stemming_strategy(stem_strategy
strategy) ";

%feature("docstring")  Xapian::QueryParser::set_stopper "

Set the stopper.

void Xapian::QueryParser::set_stopper(const Stopper *stop=NULL) ";

%feature("docstring")  Xapian::QueryParser::set_default_op "

Set the default boolean operator.

void Xapian::QueryParser::set_default_op(Query::op default_op) ";

%feature("docstring")  Xapian::QueryParser::get_default_op "

Get the default boolean operator.

Query::op Xapian::QueryParser::get_default_op() const ";

%feature("docstring")  Xapian::QueryParser::set_database "

Specify the database being searched.

void Xapian::QueryParser::set_database(const Database &db) ";

%feature("docstring")  Xapian::QueryParser::parse_query "

Parse a query.

Query Xapian::QueryParser::parse_query(const std::string
&query_string, unsigned flags=FLAG_PHRASE|FLAG_BOOLEAN|FLAG_LOVEHATE,
const std::string &default_prefix=\"\")

Parameters:
-----------

query_string:  A free-text query as entered by a user

flags:  Zero or more Query::feature_flag specifying what features the
QueryParser should support. Combine multiple values with bitwise-or
(|).

default_prefix:  The default term prefix to use (default none). For
example, you can pass \"A\" when parsing an \"Author\" field. ";

%feature("docstring")  Xapian::QueryParser::add_prefix "

Add a probabilistic term prefix.

void Xapian::QueryParser::add_prefix(const std::string &field, const
std::string &prefix)

E.g. qp.add_prefix(\"author\", \"A\");

Allows the user to search for author:orwell which will search for the
term \"Aorwel\" (assuming English stemming is in use). Multiple fields
can be mapped to the same prefix (so you can e.g. make title: and
subject: aliases for each other).

Parameters:
-----------

field:  The user visible field name

prefix:  The term prefix to map this to ";

%feature("docstring")  Xapian::QueryParser::add_boolean_prefix "

Add a boolean term prefix allowing the user to restrict a search with
a boolean filter specified in the free text query.

void Xapian::QueryParser::add_boolean_prefix(const std::string &field,
const std::string &prefix)

E.g. qp.add_boolean_prefix(\"site\", \"H\");

Allows the user to restrict a search with site:xapian.org which will
be converted to Hxapian.org combined with any probabilistic query with
OP_FILTER.

Multiple fields can be mapped to the same prefix (so you can e.g. make
site: and domain: aliases for each other).

Parameters:
-----------

field:  The user visible field name

prefix:  The term prefix to map this to ";

%feature("docstring")  Xapian::QueryParser::stoplist_begin "

Iterate over terms omitted from the query as stopwords.

TermIterator Xapian::QueryParser::stoplist_begin() const ";

%feature("docstring")  Xapian::QueryParser::stoplist_end "TermIterator Xapian::QueryParser::stoplist_end() const ";

%feature("docstring")  Xapian::QueryParser::unstem_begin "

Iterate over unstemmed forms of the given (stemmed) term used in the
query.

TermIterator Xapian::QueryParser::unstem_begin(const std::string
&term) const ";

%feature("docstring")  Xapian::QueryParser::unstem_end "TermIterator
Xapian::QueryParser::unstem_end(const std::string &) const ";

%feature("docstring")  Xapian::QueryParser::add_valuerangeprocessor "

Register a ValueRangeProcessor.

void Xapian::QueryParser::add_valuerangeprocessor(Xapian::ValueRangePr
ocessor *vrproc) ";

%feature("docstring")  Xapian::QueryParser::get_description "

Return a string describing this object.

std::string Xapian::QueryParser::get_description() const ";


// File: classXapian_1_1SimpleStopper.xml
%feature("docstring") Xapian::SimpleStopper "

Simple implementation of Stopper class - this will suit most users. ";

%feature("docstring")  Xapian::SimpleStopper::SimpleStopper "

Default constructor.

Xapian::SimpleStopper::SimpleStopper() ";

%feature("docstring")  Xapian::SimpleStopper::SimpleStopper "

Initialise from a pair of iterators.

Xapian::SimpleStopper::SimpleStopper(Iterator begin, Iterator end) ";

%feature("docstring")  Xapian::SimpleStopper::add "

Add a single stop word.

void Xapian::SimpleStopper::add(const std::string &word) ";

%feature("docstring")  Xapian::SimpleStopper::~SimpleStopper "

Destructor.

virtual Xapian::SimpleStopper::~SimpleStopper() ";

%feature("docstring")  Xapian::SimpleStopper::get_description "

Return a string describing this object.

virtual std::string Xapian::SimpleStopper::get_description() const ";


// File: classXapian_1_1Stem.xml
%feature("docstring") Xapian::Stem "

Class representing a stemming algorithm. ";

%feature("docstring")  Xapian::Stem::Stem "

Copy constructor.

Xapian::Stem::Stem(const Stem &o) ";

%feature("docstring")  Xapian::Stem::Stem "

Construct a Xapian::Stem object which doesn't change terms.

Xapian::Stem::Stem()

Equivalent to Stem(\"none\"). ";

%feature("docstring")  Xapian::Stem::Stem "

Construct a Xapian::Stem object for a particular language.

Xapian::Stem::Stem(const std::string &language)

Parameters:
-----------

language:  Either the English name for the language or the two letter
ISO639 code.

The following language names are understood (aliases follow the name):

none - don't stem terms

danish (da)

dutch (nl)

english (en) - Martin Porter's 2002 revision of his stemmer

english_lovins (lovins) - Lovin's stemmer

english_porter (porter) - Porter's stemmer as described in his 1980
paper

finnish (fi)

french (fr)

german (de)

italian (it)

norwegian (no)

portuguese (pt)

russian (ru)

spanish (es)

swedish (sv)

Parameters:
-----------

Xapian::InvalidArgumentError:  is thrown if language isn't recognised.
";

%feature("docstring")  Xapian::Stem::~Stem "

Destructor.

Xapian::Stem::~Stem() ";

%feature("docstring")  Xapian::Stem::XAPIAN_DEPRECATED "

For compatibility with Xapian 0.8.5 and earlier.

Xapian::Stem::XAPIAN_DEPRECATED(std::string stem_word(const
std::string &word) const)

Deprecated This method is now deprecated, use operator() instead. ";

%feature("docstring")  Xapian::Stem::get_description "

Return a string describing this object.

std::string Xapian::Stem::get_description() const ";


// File: classXapian_1_1Stopper.xml
%feature("docstring") Xapian::Stopper "

Base class for stop-word decision functor. ";

%feature("docstring")  Xapian::Stopper::~Stopper "

Class has virtual methods, so provide a virtual destructor.

virtual Xapian::Stopper::~Stopper() ";

%feature("docstring")  Xapian::Stopper::get_description "

Return a string describing this object.

virtual std::string Xapian::Stopper::get_description() const ";


// File: classXapian_1_1StringValueRangeProcessor.xml
%feature("docstring") Xapian::StringValueRangeProcessor "";

%feature("docstring")
Xapian::StringValueRangeProcessor::StringValueRangeProcessor "Xapian:
:StringValueRangeProcessor::StringValueRangeProcessor(Xapian::valueno
valno_) ";


// File: classXapian_1_1TermIterator.xml
%feature("docstring") Xapian::TermIterator "

An iterator pointing to items in a list of terms. ";

%feature("docstring")  Xapian::TermIterator::TermIterator "Xapian::TermIterator::TermIterator(Internal *internal_) ";

%feature("docstring")  Xapian::TermIterator::TermIterator "

Default constructor - for declaring an uninitialised iterator.

Xapian::TermIterator::TermIterator() ";

%feature("docstring")  Xapian::TermIterator::~TermIterator "

Destructor.

Xapian::TermIterator::~TermIterator() ";

%feature("docstring")  Xapian::TermIterator::TermIterator "

Copying is allowed.

Xapian::TermIterator::TermIterator(const TermIterator &other)

The internals are reference counted, so copying is also cheap. ";

%feature("docstring")  Xapian::TermIterator::skip_to "

Skip the iterator to term tname, or the first term after tname if
tname isn't in the list of terms being iterated.

void Xapian::TermIterator::skip_to(const std::string &tname) ";

%feature("docstring")  Xapian::TermIterator::get_wdf "

Return the wdf of the current term (if meaningful).

Xapian::termcount Xapian::TermIterator::get_wdf() const ";

%feature("docstring")  Xapian::TermIterator::get_termfreq "

Return the term frequency of the current term (if meaningful).

Xapian::doccount Xapian::TermIterator::get_termfreq() const ";

%feature("docstring")  Xapian::TermIterator::positionlist_count "

Return length of positionlist for current term.

Xapian::termcount Xapian::TermIterator::positionlist_count() const ";

%feature("docstring")  Xapian::TermIterator::positionlist_begin "

Return PositionIterator pointing to start of positionlist for current
term.

PositionIterator Xapian::TermIterator::positionlist_begin() const ";

%feature("docstring")  Xapian::TermIterator::positionlist_end "

Return PositionIterator pointing to end of positionlist for current
term.

PositionIterator Xapian::TermIterator::positionlist_end() const ";

%feature("docstring")  Xapian::TermIterator::get_description "

Returns a string describing this object.

std::string Xapian::TermIterator::get_description() const

Introspection method. ";


// File: classXapian_1_1TermNameWrapper.xml
%feature("docstring") Xapian::TermNameWrapper "

A wrapper class for a termname which returns the termname if
dereferenced with *.

We need this to implement input_iterator semantics. ";

%feature("docstring")  Xapian::TermNameWrapper::TermNameWrapper "Xapian::TermNameWrapper::TermNameWrapper(const std::string &tname_) ";


// File: classXapian_1_1TermPosWrapper.xml
%feature("docstring") Xapian::TermPosWrapper "

A wrapper class for a termpos which returns the termpos if
dereferenced with *.

We need this to implement input_iterator semantics. ";

%feature("docstring")  Xapian::TermPosWrapper::TermPosWrapper "Xapian::TermPosWrapper::TermPosWrapper(termpos pos_) ";


// File: classXapian_1_1ValueIterator.xml
%feature("docstring") Xapian::ValueIterator "

An iterator pointing to values associated with a document. ";

%feature("docstring")  Xapian::ValueIterator::ValueIterator "

Create an uninitialised iterator; this cannot be used, but is
convenient syntactically.

Xapian::ValueIterator::ValueIterator() ";

%feature("docstring")  Xapian::ValueIterator::~ValueIterator "Xapian::ValueIterator::~ValueIterator() ";

%feature("docstring")  Xapian::ValueIterator::ValueIterator "

Copying is allowed (and is cheap).

Xapian::ValueIterator::ValueIterator(const ValueIterator &other) ";

%feature("docstring")  Xapian::ValueIterator::get_valueno "

Get the number of the value at the current position.

Xapian::valueno Xapian::ValueIterator::get_valueno() const ";

%feature("docstring")  Xapian::ValueIterator::get_description "

Returns a string describing this object.

std::string Xapian::ValueIterator::get_description() const

Introspection method. ";


// File: structXapian_1_1ValueRangeProcessor.xml
%feature("docstring") Xapian::ValueRangeProcessor "";

%feature("docstring")
Xapian::ValueRangeProcessor::~ValueRangeProcessor "virtual
Xapian::ValueRangeProcessor::~ValueRangeProcessor() ";


// File: namespaceXapian.xml


// File: namespaceXapian_1_1Auto.xml


// File: namespaceXapian_1_1Flint.xml


// File: namespaceXapian_1_1InMemory.xml


// File: namespaceXapian_1_1Internal.xml


// File: namespaceXapian_1_1Quartz.xml


// File: namespaceXapian_1_1Remote.xml


// File: xapian_8h.xml


// File: base_8h.xml


// File: database_8h.xml


// File: dbfactory_8h.xml


// File: deprecated_8h.xml


// File: document_8h.xml


// File: enquire_8h.xml


// File: errorhandler_8h.xml


// File: expanddecider_8h.xml


// File: positioniterator_8h.xml


// File: postingiterator_8h.xml


// File: query_8h.xml


// File: queryparser_8h.xml


// File: stem_8h.xml


// File: termiterator_8h.xml


// File: types_8h.xml


// File: valueiterator_8h.xml


// File: visibility_8h.xml


// File: deprecated.xml


// File: dir_3266c64df82b843989d4784d53d9644d.xml


// File: dir_7f29c84ee0c5d248a197f1a174ea56e6.xml

