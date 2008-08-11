
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

virtual FooWeight * clone() const{ return new FooWeight(param1,
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

virtual FooWeight * clone() const{ return new FooWeight(param1,
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


// File: classXapian_1_1CategorySelectMatchSpy.xml
%feature("docstring") Xapian::CategorySelectMatchSpy "

MatchSpy for classifying matching documents by their values. ";

%feature("docstring")
Xapian::CategorySelectMatchSpy::CategorySelectMatchSpy "

Default constructor.

Xapian::CategorySelectMatchSpy::CategorySelectMatchSpy() ";

%feature("docstring")
Xapian::CategorySelectMatchSpy::CategorySelectMatchSpy "

Construct a MatchSpy which classifies matching documents based on the
values in a particular slot.

Xapian::CategorySelectMatchSpy::CategorySelectMatchSpy(Xapian::valueno
valno)

Further slots can be added by calling  add_slot(). ";

%feature("docstring")
Xapian::CategorySelectMatchSpy::score_categorisation "

Return a score reflecting how \"good\" a categorisation is.

double
Xapian::CategorySelectMatchSpy::score_categorisation(Xapian::valueno
valno, double desired_no_of_categories=0.0)

If you don't want to show a poor categorisation, or have multiple
categories and only space in your user interface to show a few, you
want to be able to decide how \"good\" a categorisation is. We define
a good categorisation as one which offers a fairly even split, and
(optionally) about a specified number of options.

Parameters:
-----------

valno:  Value number to look at the categorisation for.

desired_no_of_categories:  The desired number of categories - this is
a floating point value, so you can ask for 5.5 if you'd like \"about 5
or 6 categories\". The default is to desire the number of categories
that there actually are, so the score then only reflects how even the
split is.

A score for the categorisation for value valno - lower is better, with
a perfectly even split across the right number of categories scoring
0. ";

%feature("docstring")
Xapian::CategorySelectMatchSpy::build_numeric_ranges "

Turn a category containing sort-encoded numeric values into a set of
ranges.

bool
Xapian::CategorySelectMatchSpy::build_numeric_ranges(Xapian::valueno
valno, size_t max_ranges)

For \"continuous\" values (such as price, height, weight, etc), there
will usually be too many different values to offer the user, and the
user won't want to restrict to an exact value anyway.

This method produces a set of ranges for a particular value number.
The ranges replace the category data for value valno - the keys are
either empty (entry for \"no value set\"), <= 9 bytes long (a
singleton encoded value), or > 9 bytes long (the first 9 bytes are the
encoded range start, the rest the encoded range end).

Parameters:
-----------

valno:  Value number to produce ranges for.

max_ranges:  Group into at most this many ranges.

true if ranges could be built; false if not (e.g. all values the same,
no values set, or other reasons). ";


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
(for example, a required file cannot be found).

DatabaseVersionError:  may be thrown if the database is in an
unsupported format (for example, created by a newer version of Xapian
which uses an incompatible format). ";

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

Return a string describing this object.

virtual std::string Xapian::Database::get_description() const ";

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

%feature("docstring")  Xapian::Database::allterms_begin "

An iterator which runs across all terms with a given prefix.

TermIterator Xapian::Database::allterms_begin(const std::string
&prefix) const

This is functionally similar to getting an iterator with
allterms_begin() and then calling skip_to(prefix) on that iterator to
move to the start of the prefix, but is more convenient (because it
detects the end of the prefixed terms), and may be more efficient than
simply calling skip_to() after opening the iterator, particularly for
network databases.

Parameters:
-----------

prefix:  The prefix to restrict the returned terms to. ";

%feature("docstring")  Xapian::Database::allterms_end "

Corresponding end iterator to allterms_begin(prefix).

TermIterator Xapian::Database::allterms_end(const std::string &) const
";

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

This is the sum of the number of occurrences of the term in each
document it indexes: i.e., the sum of the within document frequencies
of the term.

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

%feature("docstring")  Xapian::Database::get_spelling_suggestion "

Suggest a spelling correction.

std::string Xapian::Database::get_spelling_suggestion(const
std::string &word, unsigned max_edit_distance=2) const

Parameters:
-----------

word:  The potentially misspelled word.

max_edit_distance:  Only consider words which are at most
max_edit_distance edits from word. An edit is a character insertion,
deletion, or the transposition of two adjacent characters (default is
2). ";

%feature("docstring")  Xapian::Database::spellings_begin "

An iterator which returns all the spelling correction targets.

Xapian::TermIterator Xapian::Database::spellings_begin() const

This returns all the words which are considered as targets for the
spelling correction algorithm. The frequency of each word is available
as the term frequency of each entry in the returned iterator. ";

%feature("docstring")  Xapian::Database::spellings_end "

Corresponding end iterator to spellings_begin().

Xapian::TermIterator Xapian::Database::spellings_end() const ";

%feature("docstring")  Xapian::Database::synonyms_begin "

An iterator which returns all the synonyms for a given term.

Xapian::TermIterator Xapian::Database::synonyms_begin(const
std::string &term) const

Parameters:
-----------

term:  The term to return synonyms for. ";

%feature("docstring")  Xapian::Database::synonyms_end "

Corresponding end iterator to synonyms_begin(term).

Xapian::TermIterator Xapian::Database::synonyms_end(const std::string
&) const ";

%feature("docstring")  Xapian::Database::synonym_keys_begin "

An iterator which returns all terms which have synonyms.

Xapian::TermIterator Xapian::Database::synonym_keys_begin(const
std::string &prefix=\"\") const

Parameters:
-----------

prefix:  If non-empty, only terms with this prefix are returned. ";

%feature("docstring")  Xapian::Database::synonym_keys_end "

Corresponding end iterator to synonym_keys_begin(prefix).

Xapian::TermIterator Xapian::Database::synonym_keys_end(const
std::string &=\"\") const ";

%feature("docstring")  Xapian::Database::get_metadata "

Get the user-specified metadata associated with a given key.

std::string Xapian::Database::get_metadata(const std::string &key)
const

User-specified metadata allows you to store arbitrary information in
the form of (key,tag) pairs. See  WritableDatabase::set_metadata() for
more information.

When invoked on a Xapian::Database object representing multiple
databases, currently only the metadata for the first is considered but
this behaviour may change in the future.

If there is no piece of metadata associated with the specified key, an
empty string is returned (this applies even for backends which don't
support metadata).

Empty keys are not valid, and specifying one will cause an exception.

Parameters:
-----------

key:  The key of the metadata item to access.

The retrieved metadata item's value.

Parameters:
-----------

Xapian::InvalidArgumentError:  will be thrown if the key supplied is
empty.

Xapian::UnimplementedError:  will be thrown if the database backend in
use doesn't support user- specified metadata. ";

%feature("docstring")  Xapian::Database::metadata_keys_begin "

An iterator which returns all user-specified metadata keys.

Xapian::TermIterator Xapian::Database::metadata_keys_begin(const
std::string &prefix=\"\") const

When invoked on a Xapian::Database object representing multiple
databases, currently only the metadata for the first is considered but
this behaviour may change in the future.

Parameters:
-----------

prefix:  If non-empty, only keys with this prefix are returned. ";

%feature("docstring")  Xapian::Database::metadata_keys_end "

Corresponding end iterator to metadata_keys_begin().

Xapian::TermIterator Xapian::Database::metadata_keys_end(const
std::string &=\"\") const ";


// File: classXapian_1_1DatabaseMaster.xml
%feature("docstring") Xapian::DatabaseMaster "

Access to a master database for replication. ";

%feature("docstring")  Xapian::DatabaseMaster::DatabaseMaster "

Create a new DatabaseMaster for the database at the specified path.

Xapian::DatabaseMaster::DatabaseMaster(const std::string &path_)

The database isn't actually opened until a set of changesets is
requested. ";

%feature("docstring")  Xapian::DatabaseMaster::write_changesets_to_fd
"

Write a set of changesets for upgrading the database to a file.

void Xapian::DatabaseMaster::write_changesets_to_fd(int fd, const
std::string &start_revision, ReplicationInfo *info) const

The changesets will be such that, if they are applied in order to a
copy of the database at the start revision, a copy of the database at
the current revision (i.e. the revision which the database object is
currently open at) will be produced.

If suitable changesets have been stored in the database, this will
write the appropriate changesets, in order. If suitable changesets are
not available, this will write a copy of sufficient blocks of the
database to reconstruct the current revision.

This will therefore potentially write a very large amount of data to
the file descriptor.

Parameters:
-----------

fd:  An open file descriptor to write the changes to.

start_revision:  The starting revision of the database that the
changesets are to be applied to. Specify an empty string to get a
\"creation\" changeset, which includes the creation of the database.
The revision will include the unique identifier for the database, if
one is available.

info:  If non-NULL, the supplied structure will be updated to reflect
the changes written to the file descriptor. ";

%feature("docstring")  Xapian::DatabaseMaster::get_description "

Return a string describing this object.

std::string Xapian::DatabaseMaster::get_description() const ";


// File: classXapian_1_1DatabaseReplica.xml
%feature("docstring") Xapian::DatabaseReplica "

Access to a database replica, for applying replication to it. ";

%feature("docstring")  Xapian::DatabaseReplica::DatabaseReplica "

Copying is allowed (and is cheap).

Xapian::DatabaseReplica::DatabaseReplica(const DatabaseReplica &other)
";

%feature("docstring")  Xapian::DatabaseReplica::DatabaseReplica "

Default constructor - for declaring an uninitialised replica.

Xapian::DatabaseReplica::DatabaseReplica() ";

%feature("docstring")  Xapian::DatabaseReplica::~DatabaseReplica "

Destructor.

Xapian::DatabaseReplica::~DatabaseReplica() ";

%feature("docstring")  Xapian::DatabaseReplica::DatabaseReplica "

Open a DatabaseReplica for the database at the specified path.

Xapian::DatabaseReplica::DatabaseReplica(const std::string &path)

The path should either point to a database previously created by a
DatabaseReplica, or to a path which doesn't yet exist.

The path should always be in a directory which exists.

If the specified path does not contain a database, a database will be
created when an appropriate changeset is supplied to the replica.

Parameters:
-----------

path:  The path to make the replica at. ";

%feature("docstring")  Xapian::DatabaseReplica::set_parameter "

Set a parameter for the replica.

void Xapian::DatabaseReplica::set_parameter(const std::string &name,
const std::string &value)

This allows the parameters which were used to create the replica to be
stored, so that they can be reused in future.

Parameters:
-----------

name:  The name of the parameter to set.

value:  The value to set the parameter to. ";

%feature("docstring")  Xapian::DatabaseReplica::get_parameter "

Get a parameter from the replica.

std::string Xapian::DatabaseReplica::get_parameter(const std::string
&name) const

Parameters:
-----------

name:  The name of the parameter to get. ";

%feature("docstring")  Xapian::DatabaseReplica::get_revision_info "

Get a string describing the current revision of the replica.

std::string Xapian::DatabaseReplica::get_revision_info() const

The revision information includes a unique identifier for the master
database that the replica is of, as well as information about the
exact revision of the master database that the replica represents.
This information allows the master database to send the appropriate
changeset to mirror whatever changes have been made on the master. ";

%feature("docstring")  Xapian::DatabaseReplica::set_read_fd "

Set the file descriptor to read changesets from.

void Xapian::DatabaseReplica::set_read_fd(int fd)

This will be remembered in the DatabaseReplica, but the caller is
still responsible for closing it after it is finished with.

Parameters:
-----------

fd:  The file descriptor to read the changeset from. ";

%feature("docstring")  Xapian::DatabaseReplica::apply_next_changeset "

Read and apply the next changeset.

bool Xapian::DatabaseReplica::apply_next_changeset(ReplicationInfo
*info)

If no changesets are found on the file descriptor, returns false
immediately.

If any changesets are found on the file descriptor, exactly one of
them is applied.

A common way to use this method is to call it repeatedly until it
returns false, with an appropriate gap between each call.

Information beyond the end of the next changeset may be read from the
file descriptor and cached in the DatabaseReplica object. Therefore,
the file descriptor shouldn't be accessed by any other external code,
since it will be in an indeterminate state.

Note that if this raises an exception (other than
DatabaseCorruptError) the database will be left in a valid and
consistent state. It may or may not be changed from its initial state,
and may or may not be fully synchronised with the master database.

Parameters:
-----------

info:  If non-NULL, the supplied structure will be updated to reflect
the changes read from the file descriptor.

true if there are more changesets to apply on the file descriptor,
false otherwise. ";

%feature("docstring")  Xapian::DatabaseReplica::close "

Close the DatabaseReplica.

void Xapian::DatabaseReplica::close()

After this has been called, there will no longer be a write lock on
the database created by the DatabaseReplica, and if any of the methods
of this object which access the database are called, they will throw
an InvalidOperationError. ";

%feature("docstring")  Xapian::DatabaseReplica::get_description "

Return a string describing this object.

std::string Xapian::DatabaseReplica::get_description() const ";


// File: classXapian_1_1DateValueRangeProcessor.xml
%feature("docstring") Xapian::DateValueRangeProcessor "

Handle a date range.

Begin and end must be dates in a recognised format. ";

%feature("docstring")
Xapian::DateValueRangeProcessor::DateValueRangeProcessor "

Constructor.

Xapian::DateValueRangeProcessor::DateValueRangeProcessor(Xapian::value
no valno_, bool prefer_mdy_=false, int epoch_year_=1970)

Parameters:
-----------

valno_:  The value number to return from operator().

prefer_mdy_:  Should ambiguous dates be interpreted as month/day/year
rather than day/month/year? (default: false)

epoch_year_:  Year to use as the epoch for dates with 2 digit years
(default: 1970, so 1/1/69 is 2069 while 1/1/70 is 1970). ";


// File: classXapian_1_1DocIDWrapper.xml
%feature("docstring") Xapian::DocIDWrapper "";

%feature("docstring")  Xapian::DocIDWrapper::DocIDWrapper "Xapian::DocIDWrapper::DocIDWrapper(docid did_) ";


// File: classXapian_1_1Document.xml
%feature("docstring") Xapian::Document "

A document in the database - holds data, values, terms, and postings.
";

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

std::string Xapian::Document::get_value(Xapian::valueno valueno) const

Returns an empty string if no value with the given number is present
in the document.

Parameters:
-----------

valueno:  The number of the value. ";

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

The length of the termlist - i.e.

Xapian::termcount Xapian::Document::termlist_count() const

the number of different terms which index this document. ";

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

%feature("docstring")  Xapian::Document::get_docid "

Get the document id which is associated with this document (if any).

docid Xapian::Document::get_docid() const

NB If multiple databases are being searched together, then this will
be the document id in the individual database, not the merged
database!

If this document came from a database, return the document id in that
database. Otherwise, return 0. ";

%feature("docstring")  Xapian::Document::get_description "

Return a string describing this object.

std::string Xapian::Document::get_description() const ";


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

Copying is allowed (and is cheap).

Xapian::Enquire::Enquire(const Enquire &other) ";

%feature("docstring")  Xapian::Enquire::Enquire "

Create a Xapian::Enquire object.

Xapian::Enquire::Enquire(const Database &database, ErrorHandler
*errorhandler_=0)

This specification cannot be changed once the Xapian::Enquire is
opened: you must create a new Xapian::Enquire object to access a
different database, or set of databases.

The database supplied must have been initialised (ie, must not be the
result of calling the Database::Database() constructor). If you need
to handle a situation where you have no index gracefully, a database
created with InMemory::open() can be passed here, which represents a
completely empty database.

Parameters:
-----------

database:  Specification of the database or databases to use.

errorhandler_:  A pointer to the error handler to use. Ownership of
the object pointed to is not assumed by the Xapian::Enquire object -
the user should delete the Xapian::ErrorHandler object after the
Xapian::Enquire object is deleted. To use no error handler, this
parameter should be 0.

Parameters:
-----------

Xapian::InvalidArgumentError:  will be thrown if an initialised
Database object is supplied. ";

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

collapse_key:  value number to collapse on - at most one MSet entry
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

%feature("docstring")  Xapian::Enquire::set_cutoff "

Set the percentage and/or weight cutoffs.

void Xapian::Enquire::set_cutoff(Xapian::percent percent_cutoff,
Xapian::weight weight_cutoff=0)

Parameters:
-----------

percent_cutoff:  Minimum percentage score for returned documents. If a
document has a lower percentage score than this, it will not appear in
the MSet. If your intention is to return only matches which contain
all the terms in the query, then it's more efficient to use
Xapian::Query::OP_AND instead of Xapian::Query::OP_OR in the query
than to use set_cutoff(100). (default 0 => no percentage cut-off).

weight_cutoff:  Minimum weight for a document to be returned. If a
document has a lower score that this, it will not appear in the MSet.
It is usually only possible to choose an appropriate weight for cutoff
based on the results of a previous run of the same query; this is thus
mainly useful for alerting operations. The other potential use is with
a user specified weighting scheme. (default 0 => no weight cut-off).
";

%feature("docstring")  Xapian::Enquire::set_sort_by_relevance "

Set the sorting to be by relevance only.

void Xapian::Enquire::set_sort_by_relevance()

This is the default. ";

%feature("docstring")  Xapian::Enquire::set_sort_by_value "

Set the sorting to be by value only.

void Xapian::Enquire::set_sort_by_value(Xapian::valueno sort_key, bool
ascending=true)

NB sorting of values uses a string comparison, so you'll need to store
numbers padded with leading zeros or spaces, or with the number of
digits prepended.

Parameters:
-----------

sort_key:  value number to sort on.

ascending:  If true, documents values which sort higher by string
compare are better. If false, the sort order is reversed. (default
true) ";

%feature("docstring")  Xapian::Enquire::set_sort_by_key "

Set the sorting to be by key generated from values only.

void Xapian::Enquire::set_sort_by_key(Xapian::Sorter *sorter, bool
ascending=true)

Parameters:
-----------

sorter:  The functor to use for generating keys.

ascending:  If true, documents values which sort higher by string
compare are better. If false, the sort order is reversed. (default
true) ";

%feature("docstring")
Xapian::Enquire::set_sort_by_value_then_relevance "

Set the sorting to be by value, then by relevance for documents with
the same value.

void Xapian::Enquire::set_sort_by_value_then_relevance(Xapian::valueno
sort_key, bool ascending=true)

NB sorting of values uses a string comparison, so you'll need to store
numbers padded with leading zeros or spaces, or with the number of
digits prepended.

Parameters:
-----------

sort_key:  value number to sort on.

ascending:  If true, documents values which sort higher by string
compare are better. If false, the sort order is reversed. (default
true) ";

%feature("docstring")  Xapian::Enquire::set_sort_by_key_then_relevance
"

Set the sorting to be by keys generated from values, then by relevance
for documents with identical keys.

void Xapian::Enquire::set_sort_by_key_then_relevance(Xapian::Sorter
*sorter, bool ascending=true)

Parameters:
-----------

sorter:  The functor to use for generating keys.

ascending:  If true, keys which sort higher by string compare are
better. If false, the sort order is reversed. (default true) ";

%feature("docstring")
Xapian::Enquire::set_sort_by_relevance_then_value "

Set the sorting to be by relevance then value.

void Xapian::Enquire::set_sort_by_relevance_then_value(Xapian::valueno
sort_key, bool ascending=true)

NB sorting of values uses a string comparison, so you'll need to store
numbers padded with leading zeros or spaces, or with the number of
digits prepended.

Note that with the default BM25 weighting scheme parameters, non-
identical documents will rarely have the same weight, so this setting
will give very similar results to set_sort_by_relevance(). It becomes
more useful with particular BM25 parameter settings (e.g.
BM25Weight(1,0,1,0,0)) or custom weighting schemes.

Parameters:
-----------

sort_key:  value number to sort on.

ascending:  If true, documents values which sort higher by string
compare are better. If false, the sort order is reversed. (default
true) ";

%feature("docstring")  Xapian::Enquire::set_sort_by_relevance_then_key
"

Set the sorting to be by relevance, then by keys generated from
values.

void Xapian::Enquire::set_sort_by_relevance_then_key(Xapian::Sorter
*sorter, bool ascending=true)

Note that with the default BM25 weighting scheme parameters, non-
identical documents will rarely have the same weight, so this setting
will give very similar results to set_sort_by_relevance(). It becomes
more useful with particular BM25 parameter settings (e.g.
BM25Weight(1,0,1,0,0)) or custom weighting schemes.

Parameters:
-----------

sorter:  The functor to use for generating keys.

ascending:  If true, keys which sort higher by string compare are
better. If false, the sort order is reversed. (default true) ";

%feature("docstring")  Xapian::Enquire::get_mset "

Get (a portion of) the match set for the current query.

MSet Xapian::Enquire::get_mset(Xapian::doccount first,
Xapian::doccount maxitems, Xapian::doccount checkatleast=0, const RSet
*omrset=0, const MatchDecider *mdecider=0, const MatchDecider
*matchspy=0) const

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
forces it to consider at least this many matches and so allows for
reliable paging links.

omrset:  the relevance set to use when performing the query.

mdecider:  a decision functor to use to decide whether a given
document should be put in the MSet.

matchspy:  a decision functor to use to decide whether a given
document should be put in the MSet. The matchspy is applied to every
document which is a potential candidate for the MSet, so if there are
checkatleast or more such documents, the matchspy will see at least
checkatleast. The mdecider is assumed to be a relatively expensive
test so may be applied in a lazier fashion.

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
Xapian::Enquire::INCLUDE_QUERY_TERMS query terms may be returned from
expand

Xapian::Enquire::USE_EXACT_TERMFREQ for multi dbs, calculate the exact
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
between the time it is returned in an MSet, and the time that this
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

%feature("docstring")  Xapian::Enquire::get_description "

Return a string describing this object.

std::string Xapian::Enquire::get_description() const ";


// File: classXapian_1_1ErrorHandler.xml
%feature("docstring") Xapian::ErrorHandler "

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

%feature("docstring")  Xapian::ErrorHandler::ErrorHandler "

Default constructor.

Xapian::ErrorHandler::ErrorHandler() ";

%feature("docstring")  Xapian::ErrorHandler::~ErrorHandler "

We require a virtual destructor because we have virtual methods.

virtual Xapian::ErrorHandler::~ErrorHandler() ";


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

Return a string describing this object.

std::string Xapian::ESet::get_description() const ";


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

Return a string describing this object.

std::string Xapian::ESetIterator::get_description() const ";


// File: classXapian_1_1ExpandDecider.xml
%feature("docstring") Xapian::ExpandDecider "

Virtual base class for expand decider functor. ";

%feature("docstring")  Xapian::ExpandDecider::~ExpandDecider "

Virtual destructor, because we have virtual methods.

virtual Xapian::ExpandDecider::~ExpandDecider() ";


// File: classXapian_1_1ExpandDeciderAnd.xml
%feature("docstring") Xapian::ExpandDeciderAnd "

ExpandDecider subclass which rejects terms using two ExpandDeciders.

Terms are only accepted if they are accepted by both of the specified
ExpandDecider objects. ";

%feature("docstring")  Xapian::ExpandDeciderAnd::ExpandDeciderAnd "

Terms will be checked with first, and if accepted, then checked with
second.

Xapian::ExpandDeciderAnd::ExpandDeciderAnd(const ExpandDecider
&first_, const ExpandDecider &second_) ";

%feature("docstring")  Xapian::ExpandDeciderAnd::ExpandDeciderAnd "

Compatibility method.

Xapian::ExpandDeciderAnd::ExpandDeciderAnd(const ExpandDecider
*first_, const ExpandDecider *second_) ";


// File: classXapian_1_1ExpandDeciderFilterTerms.xml
%feature("docstring") Xapian::ExpandDeciderFilterTerms "

ExpandDecider subclass which rejects terms in a specified list.

ExpandDeciderFilterTerms provides an easy way to filter out terms from
a fixed list when generating an ESet. ";

%feature("docstring")
Xapian::ExpandDeciderFilterTerms::ExpandDeciderFilterTerms "

The two iterators specify a list of terms to be rejected.

Xapian::ExpandDeciderFilterTerms::ExpandDeciderFilterTerms(Iterator
reject_begin, Iterator reject_end)

reject_begin and reject_end can be any input_iterator type which
returns std::string or char * (e.g. TermIterator or char **). ";


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

Fetch the document info for a set of items in the MSet.

void Xapian::MSet::fetch(const MSetIterator &begin, const MSetIterator
&end) const

This method causes the documents in the range specified by the
iterators to be fetched from the database, and cached in the
Xapian::MSet object. This has little effect when performing a search
across a local database, but will greatly speed up subsequent access
to the document contents when the documents are stored in a remote
database.

The iterators must be over this Xapian::MSet - undefined behaviour
will result otherwise.

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

This is sometimes more efficient than asking the database directly for
the term frequency - in particular, if the term was in the query, its
frequency will usually be cached in the MSet. ";

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
result being the first item in the MSet. ";

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

The maximum possible weight in the MSet.

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

Return a string describing this object.

std::string Xapian::MSet::get_description() const ";


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

Return a string describing this object.

std::string Xapian::MSetIterator::get_description() const ";


// File: classXapian_1_1MultipleMatchDecider.xml
%feature("docstring") Xapian::MultipleMatchDecider "

Class which applies several match deciders in turn. ";

%feature("docstring")  Xapian::MultipleMatchDecider::append "

Add a match decider to the end of the list to be called.

void Xapian::MultipleMatchDecider::append(const MatchDecider *decider)

Note that the caller must ensure that the decider is not deleted
before it is used - the MultipleMatchDecider keeps a pointer to the
supplied decider. ";


// File: classXapian_1_1MultiValueSorter.xml
%feature("docstring") Xapian::MultiValueSorter "

Sorter subclass which sorts by a several values.

Results are ordered by the first value. In the event of a tie, the
second is used. If this is the same for both, the third is used, and
so on. ";

%feature("docstring")  Xapian::MultiValueSorter::MultiValueSorter "Xapian::MultiValueSorter::MultiValueSorter() ";

%feature("docstring")  Xapian::MultiValueSorter::MultiValueSorter "Xapian::MultiValueSorter::MultiValueSorter(Iterator begin, Iterator
end) ";

%feature("docstring")  Xapian::MultiValueSorter::add "void
Xapian::MultiValueSorter::add(Xapian::valueno valno, bool
forward=true) ";

%feature("docstring")  Xapian::MultiValueSorter::~MultiValueSorter "virtual Xapian::MultiValueSorter::~MultiValueSorter() ";


// File: classXapian_1_1NumberValueRangeProcessor.xml
%feature("docstring") Xapian::NumberValueRangeProcessor "

Handle a number range.

This class must be used on values which have been encoded using
Xapian::sortable_serialise() which turns numbers into strings which
will sort in the same order as the numbers (the same values can be
used to implement a numeric sort). ";

%feature("docstring")
Xapian::NumberValueRangeProcessor::NumberValueRangeProcessor "

Constructor.

Xapian::NumberValueRangeProcessor::NumberValueRangeProcessor(Xapian::v
alueno valno_)

Parameters:
-----------

valno_:  The value number to return from operator(). ";

%feature("docstring")
Xapian::NumberValueRangeProcessor::NumberValueRangeProcessor "

Constructor.

Xapian::NumberValueRangeProcessor::NumberValueRangeProcessor(Xapian::v
alueno valno_, const std::string &str_, bool prefix_=true)

Parameters:
-----------

valno_:  The value number to return from operator().

str_:  A string to look for to recognise values as belonging to this
numeric range.

prefix_:  Whether to look for the string at the start or end of the
values. If true, the string is a prefix; if false, the string is a
suffix (default: true).

The string supplied in str_ is used by operator() to decide whether
the pair of strings supplied to it constitute a valid range. If
prefix_ is true, the first value in a range must begin with str_ (and
the second value may optionally begin with str_); if prefix_ is false,
the second value in a range must end with str_ (and the first value
may optionally end with str_).

If str_ is empty, the setting of prefix_ is irrelevant, and no special
strings are required at the start or end of the strings defining the
range.

The remainder of both strings defining the endpoints must be valid
floating point numbers. (FIXME: define format recognised).

For example, if str_ is \"$\" and prefix_ is true, and the range
processor has been added to the queryparser, the queryparser will
accept \"$10..50\" or \"$10..$50\", but not \"10..50\" or \"10..$50\"
as valid ranges. If str_ is \"kg\" and prefix_ is false, the
queryparser will accept \"10..50kg\" or \"10kg..50kg\", but not
\"10..50\" or \"10kg..50\" as valid ranges. ";


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

Return a string describing this object.

std::string Xapian::PositionIterator::get_description() const ";


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

Return a string describing this object.

std::string Xapian::PostingIterator::get_description() const ";


// File: classXapian_1_1PostingSource.xml
%feature("docstring") Xapian::PostingSource "

Base class which provides an \"external\" source of postings. ";

%feature("docstring")  Xapian::PostingSource::~PostingSource "virtual
Xapian::PostingSource::~PostingSource() ";

%feature("docstring")  Xapian::PostingSource::get_termfreq_min "virtual Xapian::doccount Xapian::PostingSource::get_termfreq_min()
const =0 ";

%feature("docstring")  Xapian::PostingSource::get_termfreq_est "virtual Xapian::doccount Xapian::PostingSource::get_termfreq_est()
const =0 ";

%feature("docstring")  Xapian::PostingSource::get_termfreq_max "virtual Xapian::doccount Xapian::PostingSource::get_termfreq_max()
const =0 ";

%feature("docstring")  Xapian::PostingSource::get_maxweight "

This default implementation always returns 0, for convenience when
implementing \"weight-less\" PostingSource subclasses.

virtual Xapian::weight Xapian::PostingSource::get_maxweight() const ";

%feature("docstring")  Xapian::PostingSource::get_weight "

This default implementation always returns 0, for convenience when
implementing \"weight-less\" PostingSource subclasses.

virtual Xapian::weight Xapian::PostingSource::get_weight() const ";

%feature("docstring")  Xapian::PostingSource::next "virtual void
Xapian::PostingSource::next(Xapian::weight)=0 ";

%feature("docstring")  Xapian::PostingSource::skip_to "

This default implementation calls next() repeatedly.

virtual void Xapian::PostingSource::skip_to(Xapian::docid,
Xapian::weight) ";

%feature("docstring")  Xapian::PostingSource::check "

This default implementation calls skip_to() and always sets valid to
true.

virtual void Xapian::PostingSource::check(Xapian::docid,
Xapian::weight, bool &) ";

%feature("docstring")  Xapian::PostingSource::at_end "virtual bool
Xapian::PostingSource::at_end() const =0 ";

%feature("docstring")  Xapian::PostingSource::get_docid "virtual
Xapian::docid Xapian::PostingSource::get_docid() const =0 ";

%feature("docstring")  Xapian::PostingSource::reset "virtual void
Xapian::PostingSource::reset()=0 ";

%feature("docstring")  Xapian::PostingSource::get_description "

This default implementation returns a generic answer.

virtual std::string Xapian::PostingSource::get_description() const ";


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

Apply the specified operator to a single Xapian::Query object, with a
double parameter.

Xapian::Query::Query(Query::op op_, Xapian::Query q, double parameter)
";

%feature("docstring")  Xapian::Query::Query "

Construct a value range query on a document value.

Xapian::Query::Query(Query::op op_, Xapian::valueno valno, const
std::string &begin, const std::string &end)

A value range query matches those documents which have a value stored
in the slot given by valno which is in the range specified by begin
and end (in lexicographical order), including the endpoints.

Parameters:
-----------

op_:  The operator to use for the query. Currently, must be
OP_VALUE_RANGE.

valno:  The slot number to get the value from.

begin:  The start of the range.

end:  The end of the range. ";

%feature("docstring")  Xapian::Query::Query "

Construct a value comparison query on a document value.

Xapian::Query::Query(Query::op op_, Xapian::valueno valno, const
std::string &value)

This query matches those documents which have a value stored in the
slot given by valno which compares, as specified by the operator, to
value.

Parameters:
-----------

op_:  The operator to use for the query. Currently, must be
OP_VALUE_GE or OP_VALUE_LE.

valno:  The slot number to get the value from.

value:  The value to compare. ";

%feature("docstring")  Xapian::Query::Query "

Construct an external source query.

Xapian::Query::Query(Xapian::PostingSource *external_source) ";

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

%feature("docstring")  Xapian::Query::get_description "

Return a string describing this object.

std::string Xapian::Query::get_description() const ";

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

%feature("docstring")  Xapian::Query::Internal "

Construct a value greater-than-or-equal query on a document value.

Xapian::Query::Internal(op_t op_, Xapian::valueno valno, const
std::string &value) ";

%feature("docstring")  Xapian::Query::Internal "

Construct an external source query.

Xapian::Query::Internal(Xapian::PostingSource *external_source_) ";

%feature("docstring")  Xapian::Query::~Internal "

Destructor.

Xapian::Query::~Internal() ";

%feature("docstring")  Xapian::Query::add_subquery "

Add a subquery.

void Xapian::Query::add_subquery(const Query::Internal *subq) ";

%feature("docstring")  Xapian::Query::set_dbl_parameter "void
Xapian::Query::set_dbl_parameter(double dbl_parameter_) ";

%feature("docstring")  Xapian::Query::get_dbl_parameter "double
Xapian::Query::get_dbl_parameter() const ";

%feature("docstring")  Xapian::Query::end_construction "

Finish off the construction.

Query::Internal* Xapian::Query::end_construction() ";

%feature("docstring")  Xapian::Query::serialise "

Return a string in an easily parsed form which contains all the
information in a query.

std::string Xapian::Query::serialise() const ";

%feature("docstring")  Xapian::Query::get_description "

Return a string describing this object.

std::string Xapian::Query::get_description() const ";

%feature("docstring")  Xapian::Query::get_parameter "

Get the numeric parameter used in this query.

Xapian::termcount Xapian::Query::get_parameter() const

This is used by the QueryParser to get the value number for
VALUE_RANGE queries. It should be replaced by a public method on the
Query class at some point, but the API which should be used for that
is unclear, so this is a temporary workaround. ";

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

void Xapian::QueryParser::set_stemmer(const Xapian::Stem &stemmer)

This sets the stemming algorithm which will be used by the query
parser. Note that the stemming algorithm will only be used according
to the stemming strategy set by set_stemming_strategy(), which
defaults to STEM_NONE. Therefore, to use a stemming algorithm, you
will also need to call set_stemming_strategy() with a value other than
STEM_NONE. ";

%feature("docstring")  Xapian::QueryParser::set_stemming_strategy "

Set the stemming strategy.

void Xapian::QueryParser::set_stemming_strategy(stem_strategy
strategy)

This controls how the query parser will apply the stemming algorithm.
The default value is STEM_NONE. The possible values are:

STEM_NONE: Don't perform any stemming.

STEM_SOME: Search for stemmed forms of terms except for those which
start with a capital letter, or are followed by certain characters
(currently: (/@<>=*[{\" ), or are used with operators which need
positional information. Stemmed terms are prefixed with 'Z'.

STEM_ALL: Search for stemmed forms of all words (note: no 'Z' prefix
is added).

Note that the stemming algorithm is only applied to words in
probabilistic fields - boolean filter terms are never stemmed. ";

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

For example:

This allows the user to search for author:Orwell which will be
converted to a search for the term \"Aorwell\".

Multiple fields can be mapped to the same prefix. For example, you can
make title: and subject: aliases for each other.

As of 1.0.4, you can call this method multiple times with the same
value of field to allow a single field to be mapped to multiple
prefixes. Multiple terms being generated for such a field, and
combined with  Xapian::Query::OP_OR.

If any prefixes are specified for the empty field name (i.e. you call
this method with an empty string as the first parameter) these
prefixes will be used as the default prefix. If you do this and also
specify the default_prefix parameter to  parse_query(), then the
default_prefix parameter will override.

If you call  add_prefix() and  add_boolean_prefix() for the same value
of field, a Xapian::InvalidOperationError exception will be thrown.

In 1.0.3 and earlier, subsequent calls to this method with the same
value of field had no effect.

Parameters:
-----------

field:  The user visible field name

prefix:  The term prefix to map this to ";

%feature("docstring")  Xapian::QueryParser::add_boolean_prefix "

Add a boolean term prefix allowing the user to restrict a search with
a boolean filter specified in the free text query.

void Xapian::QueryParser::add_boolean_prefix(const std::string &field,
const std::string &prefix)

For example:

This allows the user to restrict a search with site:xapian.org which
will be converted to Hxapian.org combined with any probabilistic query
with  Xapian::Query::OP_FILTER.

If multiple boolean filters are specified in a query for the same
prefix, they will be combined with the  Xapian::Query::OP_OR operator.
Then, if there are boolean filters for different prefixes, they will
be combined with the  Xapian::Query::OP_AND operator.

Multiple fields can be mapped to the same prefix (so for example you
can make site: and domain: aliases for each other). Instances of
fields with different aliases but the same prefix will still be
combined with the OR operator.

For example, if \"site\" and \"domain\" map to \"H\", but author maps
to \"A\", a search for \"site:foo domain:bar author:Fred\" will map to
\"(Hfoo OR Hbar) AND Afred\".

As of 1.0.4, you can call this method multiple times with the same
value of field to allow a single field to be mapped to multiple
prefixes. Multiple terms being generated for such a field, and
combined with  Xapian::Query::OP_OR.

Calling this method with an empty string for field will cause a
Xapian::InvalidArgumentError.

If you call  add_prefix() and  add_boolean_prefix() for the same value
of field, a Xapian::InvalidOperationError exception will be thrown.

In 1.0.3 and earlier, subsequent calls to this method with the same
value of field had no effect.

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

%feature("docstring")  Xapian::QueryParser::get_corrected_query_string
"

Get the spelling-corrected query string.

std::string Xapian::QueryParser::get_corrected_query_string() const

This will only be set if FLAG_SPELLING_CORRECTION is specified when
QueryParser::parse_query() was last called.

If there were no corrections, an empty string is returned. ";

%feature("docstring")  Xapian::QueryParser::get_description "

Return a string describing this object.

std::string Xapian::QueryParser::get_description() const ";


// File: classXapian_1_1Internal_1_1RefCntBase.xml
%feature("docstring") Xapian::Internal::RefCntBase "";

%feature("docstring")  Xapian::Internal::RefCntBase::RefCntBase "

The constructor, which initialises the ref_count to 0.

Xapian::Internal::RefCntBase::RefCntBase() ";


// File: classXapian_1_1Internal_1_1RefCntPtr.xml
%feature("docstring") Xapian::Internal::RefCntPtr "";

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


// File: structXapian_1_1ReplicationInfo.xml
%feature("docstring") Xapian::ReplicationInfo "

Information about the steps involved in performing a replication. ";

%feature("docstring")  Xapian::ReplicationInfo::ReplicationInfo "Xapian::ReplicationInfo::ReplicationInfo() ";

%feature("docstring")  Xapian::ReplicationInfo::clear "void
Xapian::ReplicationInfo::clear() ";


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

Return a string describing this object.

std::string Xapian::RSet::get_description() const ";


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

%feature("docstring")  Xapian::SimpleStopper::get_description "

Return a string describing this object.

virtual std::string Xapian::SimpleStopper::get_description() const ";


// File: classXapian_1_1Sorter.xml
%feature("docstring") Xapian::Sorter "

Virtual base class for sorter functor. ";

%feature("docstring")  Xapian::Sorter::~Sorter "

Virtual destructor, because we have virtual methods.

virtual Xapian::Sorter::~Sorter() ";


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


// File: structXapian_1_1StringAndFrequency.xml
%feature("docstring") Xapian::StringAndFrequency "

A string with a corresponding frequency. ";

%feature("docstring")  Xapian::StringAndFrequency::StringAndFrequency
"Xapian::StringAndFrequency::StringAndFrequency(std::string str_,
Xapian::doccount frequency_) ";


// File: classXapian_1_1StringListSerialiser.xml
%feature("docstring") Xapian::StringListSerialiser "

Class to serialise a list of strings in a form suitable for
ValueCountMatchSpy. ";

%feature("docstring")
Xapian::StringListSerialiser::StringListSerialiser "

Default constructor.

Xapian::StringListSerialiser::StringListSerialiser() ";

%feature("docstring")
Xapian::StringListSerialiser::StringListSerialiser "

Initialise with a string.

Xapian::StringListSerialiser::StringListSerialiser(const std::string
&initial)

(The string represents a serialised form, rather than a single value
to be serialised.) ";

%feature("docstring")
Xapian::StringListSerialiser::StringListSerialiser "

Initialise from a pair of iterators.

Xapian::StringListSerialiser::StringListSerialiser(Iterator begin,
Iterator end) ";

%feature("docstring")  Xapian::StringListSerialiser::append "

Add a string to the end of the list.

void Xapian::StringListSerialiser::append(const std::string &value) ";

%feature("docstring")  Xapian::StringListSerialiser::get "

Get the serialised result.

const std::string& Xapian::StringListSerialiser::get() const ";


// File: classXapian_1_1StringListUnserialiser.xml
%feature("docstring") Xapian::StringListUnserialiser "

Class to unserialise a list of strings serialised by a
StringListSerialiser.

The class can be used as an iterator: use the default constructor to
get an end iterator. ";

%feature("docstring")
Xapian::StringListUnserialiser::StringListUnserialiser "

Default constructor - use this to define an end iterator.

Xapian::StringListUnserialiser::StringListUnserialiser() ";

%feature("docstring")
Xapian::StringListUnserialiser::StringListUnserialiser "

Constructor which takes a serialised list of strings, and creates an
iterator pointing to the first of them.

Xapian::StringListUnserialiser::StringListUnserialiser(const
std::string &in) ";

%feature("docstring")
Xapian::StringListUnserialiser::~StringListUnserialiser "

Destructor - nothing special to release.

Xapian::StringListUnserialiser::~StringListUnserialiser() ";

%feature("docstring")
Xapian::StringListUnserialiser::StringListUnserialiser "

Copy constructor.

Xapian::StringListUnserialiser::StringListUnserialiser(const
StringListUnserialiser &other) ";


// File: classXapian_1_1StringValueRangeProcessor.xml
%feature("docstring") Xapian::StringValueRangeProcessor "

Handle a string range.

The end points can be any strings. ";

%feature("docstring")
Xapian::StringValueRangeProcessor::StringValueRangeProcessor "

Constructor.

Xapian::StringValueRangeProcessor::StringValueRangeProcessor(Xapian::v
alueno valno_)

Parameters:
-----------

valno_:  The value number to return from operator(). ";


// File: classXapian_1_1TermCountMatchSpy.xml
%feature("docstring") Xapian::TermCountMatchSpy "

Class for counting the frequencies of terms in the matching documents.

Note that accessing the list of terms is generally more expensive than
accessing a value, so if it is possible to store the information you
need in a value, you should probably use a ValueCountMatchSpy instead
of a TermCountMatchSpy. ";

%feature("docstring")  Xapian::TermCountMatchSpy::TermCountMatchSpy "

Default constructor.

Xapian::TermCountMatchSpy::TermCountMatchSpy() ";

%feature("docstring")  Xapian::TermCountMatchSpy::TermCountMatchSpy "

Construct a MatchSpy which counts the terms with a particular prefix.

Xapian::TermCountMatchSpy::TermCountMatchSpy(std::string prefix)

Further prefixes can be added by calling  add_prefix(). ";

%feature("docstring")  Xapian::TermCountMatchSpy::add_prefix "

Add a prefix to count terms with.

void Xapian::TermCountMatchSpy::add_prefix(std::string prefix)

A TermCountMatchSpy can count terms with one or more prefixes. If the
prefixes overlap (eg, \"X\" and \"XA\"), terms with both prefixes will
be counted for each of those prefixes. ";

%feature("docstring")  Xapian::TermCountMatchSpy::get_terms "

Return the suffixes of those terms seen with prefix prefix.

const std::map<std::string, Xapian::doccount>&
Xapian::TermCountMatchSpy::get_terms(std::string prefix) const

Parameters:
-----------

prefix:  The prefix to examine (must have specified for examination
before performing the match - either by using the  add_prefix()
method, or using the constructor which takes a prefix.) ";

%feature("docstring")  Xapian::TermCountMatchSpy::get_documents_seen "

Return the number of documents tallied.

size_t Xapian::TermCountMatchSpy::get_documents_seen() const ";

%feature("docstring")  Xapian::TermCountMatchSpy::get_terms_seen "

Return the number of term occurrences tallied.

size_t Xapian::TermCountMatchSpy::get_terms_seen() const

If terms occur in more than one of the prefixes specified, they will
be counted multiple times. ";

%feature("docstring")  Xapian::TermCountMatchSpy::get_top_terms "

Get the most frequent terms with a given prefix.

void Xapian::TermCountMatchSpy::get_top_terms(std::vector<
StringAndFrequency > &result, std::string prefix, size_t maxterms)
const

Parameters:
-----------

result:  A vector which will be filled with the most frequent terms,
in descending order of frequency. Terms with the same frequency will
be sorted in ascending alphabetical order.

prefix:  The prefix to examine (must have specified for examination
before performing the match - either by using the  add_prefix()
method, or using the constructor which takes a prefix.)

maxterms:  The maximum number of terms to return. ";


// File: classXapian_1_1TermGenerator.xml
%feature("docstring") Xapian::TermGenerator "

Parses a piece of text and generate terms.

This module takes a piece of text and parses it to produce words which
are then used to generate suitable terms for indexing. The terms
generated are suitable for use with Query objects produced by the
QueryParser class. ";

%feature("docstring")  Xapian::TermGenerator::TermGenerator "

Copy constructor.

Xapian::TermGenerator::TermGenerator(const TermGenerator &o) ";

%feature("docstring")  Xapian::TermGenerator::TermGenerator "

Default constructor.

Xapian::TermGenerator::TermGenerator() ";

%feature("docstring")  Xapian::TermGenerator::~TermGenerator "

Destructor.

Xapian::TermGenerator::~TermGenerator() ";

%feature("docstring")  Xapian::TermGenerator::set_stemmer "

Set the Xapian::Stem object to be used for generating stemmed terms.

void Xapian::TermGenerator::set_stemmer(const Xapian::Stem &stemmer)
";

%feature("docstring")  Xapian::TermGenerator::set_stopper "

Set the Xapian::Stopper object to be used for identifying stopwords.

void Xapian::TermGenerator::set_stopper(const Xapian::Stopper
*stop=NULL) ";

%feature("docstring")  Xapian::TermGenerator::set_document "

Set the current document.

void Xapian::TermGenerator::set_document(const Xapian::Document &doc)
";

%feature("docstring")  Xapian::TermGenerator::get_document "

Get the current document.

const Xapian::Document& Xapian::TermGenerator::get_document() const ";

%feature("docstring")  Xapian::TermGenerator::set_database "

Set the database to index spelling data to.

void Xapian::TermGenerator::set_database(const
Xapian::WritableDatabase &db) ";

%feature("docstring")  Xapian::TermGenerator::set_flags "

Set flags.

flags Xapian::TermGenerator::set_flags(flags toggle, flags
mask=flags(0))

The new value of flags is: (flags & mask) ^ toggle

To just set the flags, pass the new flags in toggle and the default
value for mask.

Parameters:
-----------

toggle:  Flags to XOR.

mask:  Flags to AND with first.

The old flags setting. ";

%feature("docstring")  Xapian::TermGenerator::index_text "

Index some text.

void Xapian::TermGenerator::index_text(const Xapian::Utf8Iterator
&itor, Xapian::termcount weight=1, const std::string &prefix=\"\")

Parameters:
-----------

weight:  The wdf increment (default 1).

prefix:  The term prefix to use (default is no prefix). ";

%feature("docstring")  Xapian::TermGenerator::index_text "

Index some text in a std::string.

void Xapian::TermGenerator::index_text(const std::string &text,
Xapian::termcount weight=1, const std::string &prefix=\"\")

Parameters:
-----------

weight:  The wdf increment (default 1).

prefix:  The term prefix to use (default is no prefix). ";

%feature("docstring")
Xapian::TermGenerator::index_text_without_positions "

Index some text without positional information.

void Xapian::TermGenerator::index_text_without_positions(const
Xapian::Utf8Iterator &itor, Xapian::termcount weight=1, const
std::string &prefix=\"\")

Just like index_text, but no positional information is generated. This
means that the database will be significantly smaller, but that phrase
searching and NEAR won't be supported. ";

%feature("docstring")
Xapian::TermGenerator::index_text_without_positions "

Index some text in a std::string without positional information.

void Xapian::TermGenerator::index_text_without_positions(const
std::string &text, Xapian::termcount weight=1, const std::string
&prefix=\"\")

Just like index_text, but no positional information is generated. This
means that the database will be significantly smaller, but that phrase
searching and NEAR won't be supported. ";

%feature("docstring")  Xapian::TermGenerator::increase_termpos "

Increase the termpos used by index_text by delta.

void Xapian::TermGenerator::increase_termpos(Xapian::termcount
delta=100)

This can be used to prevent phrase searches from spanning two
unconnected blocks of text (e.g. the title and body text). ";

%feature("docstring")  Xapian::TermGenerator::get_termpos "

Get the current term position.

Xapian::termcount Xapian::TermGenerator::get_termpos() const ";

%feature("docstring")  Xapian::TermGenerator::set_termpos "

Set the current term position.

void Xapian::TermGenerator::set_termpos(Xapian::termcount termpos) ";

%feature("docstring")  Xapian::TermGenerator::get_description "

Return a string describing this object.

std::string Xapian::TermGenerator::get_description() const ";


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

Xapian::termcount Xapian::TermIterator::get_wdf() const

The wdf (within document frequency) is the number of occurences of a
term in a particular document. ";

%feature("docstring")  Xapian::TermIterator::get_termfreq "

Return the term frequency of the current term (if meaningful).

Xapian::doccount Xapian::TermIterator::get_termfreq() const

The term frequency is the number of documents which a term indexes. ";

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

Return a string describing this object.

std::string Xapian::TermIterator::get_description() const ";


// File: classXapian_1_1TermNameWrapper.xml
%feature("docstring") Xapian::TermNameWrapper "";

%feature("docstring")  Xapian::TermNameWrapper::TermNameWrapper "Xapian::TermNameWrapper::TermNameWrapper(const std::string &tname_) ";


// File: classXapian_1_1TermPosWrapper.xml
%feature("docstring") Xapian::TermPosWrapper "";

%feature("docstring")  Xapian::TermPosWrapper::TermPosWrapper "Xapian::TermPosWrapper::TermPosWrapper(termpos pos_) ";


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

virtual FooWeight * clone() const{ return new FooWeight(param1,
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


// File: classXapian_1_1Utf8Iterator.xml
%feature("docstring") Xapian::Utf8Iterator "

An iterator which returns unicode character values from a UTF-8
encoded string. ";

%feature("docstring")  Xapian::Utf8Iterator::raw "

Return the raw const char * pointer for the current position.

const char* Xapian::Utf8Iterator::raw() const ";

%feature("docstring")  Xapian::Utf8Iterator::left "

Return the number of bytes left in the iterator's buffer.

size_t Xapian::Utf8Iterator::left() const ";

%feature("docstring")  Xapian::Utf8Iterator::assign "

Assign a new string to the iterator.

void Xapian::Utf8Iterator::assign(const char *p_, size_t len)

The iterator will forget the string it was iterating through, and
return characters from the start of the new string when next called.
The string is not copied into the iterator, so it must remain valid
while the iteration is in progress.

Parameters:
-----------

p:  A pointer to the start of the string to read.

len:  The length of the string to read. ";

%feature("docstring")  Xapian::Utf8Iterator::assign "

Assign a new string to the iterator.

void Xapian::Utf8Iterator::assign(const std::string &s)

The iterator will forget the string it was iterating through, and
return characters from the start of the new string when next called.
The string is not copied into the iterator, so it must remain valid
while the iteration is in progress.

Parameters:
-----------

s:  The string to read. Must not be modified while the iteration is in
progress. ";

%feature("docstring")  Xapian::Utf8Iterator::Utf8Iterator "

Create an iterator given a pointer to a null terminated string.

Xapian::Utf8Iterator::Utf8Iterator(const char *p_)

The iterator will return characters from the start of the string when
next called. The string is not copied into the iterator, so it must
remain valid while the iteration is in progress.

Parameters:
-----------

p:  A pointer to the start of the null terminated string to read. ";

%feature("docstring")  Xapian::Utf8Iterator::Utf8Iterator "

Create an iterator given a pointer and a length.

Xapian::Utf8Iterator::Utf8Iterator(const char *p_, size_t len)

The iterator will return characters from the start of the string when
next called. The string is not copied into the iterator, so it must
remain valid while the iteration is in progress.

Parameters:
-----------

p:  A pointer to the start of the string to read.

len:  The length of the string to read. ";

%feature("docstring")  Xapian::Utf8Iterator::Utf8Iterator "

Create an iterator given a string.

Xapian::Utf8Iterator::Utf8Iterator(const std::string &s)

The iterator will return characters from the start of the string when
next called. The string is not copied into the iterator, so it must
remain valid while the iteration is in progress.

Parameters:
-----------

s:  The string to read. Must not be modified while the iteration is in
progress. ";

%feature("docstring")  Xapian::Utf8Iterator::Utf8Iterator "

Create an iterator which is at the end of its iteration.

Xapian::Utf8Iterator::Utf8Iterator()

This can be compared to another iterator to check if the other
iterator has reached its end. ";


// File: classXapian_1_1ValueCountMatchSpy.xml
%feature("docstring") Xapian::ValueCountMatchSpy "

Class for counting the frequencies of values in the matching
documents. ";

%feature("docstring")  Xapian::ValueCountMatchSpy::ValueCountMatchSpy
"

Default constructor.

Xapian::ValueCountMatchSpy::ValueCountMatchSpy() ";

%feature("docstring")  Xapian::ValueCountMatchSpy::ValueCountMatchSpy
"

Construct a MatchSpy which counts the values in a particular slot.

Xapian::ValueCountMatchSpy::ValueCountMatchSpy(Xapian::valueno valno,
bool multivalue=false)

Further slots can be added by calling  add_slot(). ";

%feature("docstring")  Xapian::ValueCountMatchSpy::add_slot "

Add a slot number to count values in.

void Xapian::ValueCountMatchSpy::add_slot(Xapian::valueno valno, bool
multivalue=false)

A ValueCountMatchSpy can count values in one or more slots. ";

%feature("docstring")  Xapian::ValueCountMatchSpy::get_values "

Return the values seen in slot number valno.

const std::map<std::string, Xapian::doccount>&
Xapian::ValueCountMatchSpy::get_values(Xapian::valueno valno) const

Parameters:
-----------

valno:  The slot to examine (must have specified for examination
before performing the match - either by using the  add_slot() method,
or using the constructor which takes a slot number.) ";

%feature("docstring")  Xapian::ValueCountMatchSpy::get_total "

Return the total number of documents tallied.

size_t Xapian::ValueCountMatchSpy::get_total() const ";

%feature("docstring")  Xapian::ValueCountMatchSpy::get_top_values "

Get the most frequent values in a slot.

void Xapian::ValueCountMatchSpy::get_top_values(std::vector<
StringAndFrequency > &result, Xapian::valueno valno, size_t maxvalues)
const

Parameters:
-----------

result:  A vector which will be filled with the most frequent values,
in descending order of frequency. Values with the same frequency will
be sorted in ascending alphabetical order.

valno:  The slot to examine (must have specified for examination
before performing the match - either by using the  add_slot() method,
or using the constructor which takes a slot number.)

maxvalues:  The maximum number of values to return. ";


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

Return a string describing this object.

std::string Xapian::ValueIterator::get_description() const ";


// File: structXapian_1_1ValueRangeProcessor.xml
%feature("docstring") Xapian::ValueRangeProcessor "

Base class for value range processors. ";

%feature("docstring")
Xapian::ValueRangeProcessor::~ValueRangeProcessor "

Destructor.

virtual Xapian::ValueRangeProcessor::~ValueRangeProcessor() ";


// File: classXapian_1_1ValueSetMatchDecider.xml
%feature("docstring") Xapian::ValueSetMatchDecider "

MatchDecider filtering results based on whether document values are in
a user- defined set. ";

%feature("docstring")
Xapian::ValueSetMatchDecider::ValueSetMatchDecider "

Construct a ValueSetMatchDecider.

Xapian::ValueSetMatchDecider::ValueSetMatchDecider(Xapian::valueno
valuenum, bool inclusive)

Parameters:
-----------

valuenum:  The value slot number to look in.

inclusive:  If true, match decider accepts documents which have a
value in the specified slot which is a member of the test set; if
false, match decider accepts documents which do not have a value in
the specified slot. ";

%feature("docstring")  Xapian::ValueSetMatchDecider::add_value "

Add a value to the test set.

void Xapian::ValueSetMatchDecider::add_value(const std::string &value)

Parameters:
-----------

value:  The value to add to the test set. ";

%feature("docstring")  Xapian::ValueSetMatchDecider::remove_value "

Remove a value from the test set.

void Xapian::ValueSetMatchDecider::remove_value(const std::string
&value)

Parameters:
-----------

value:  The value to remove from the test set. ";


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

virtual std::string Xapian::Weight::name() const =0

If the subclass is called FooWeight, this should return \"Foo\". ";

%feature("docstring")  Xapian::Weight::serialise "

Serialise object parameters into a string.

virtual std::string Xapian::Weight::serialise() const =0 ";

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

virtual Xapian::weight Xapian::Weight::get_maxpart() const =0

This is used in optimising searches, by having the postlist tree decay
appropriately when parts of it can have limited, or no, further
effect. ";

%feature("docstring")  Xapian::Weight::get_sumextra "

Get an extra weight for a document to add to the sum calculated over
the query terms.

virtual Xapian::weight Xapian::Weight::get_sumextra(Xapian::doclength
len) const =0

This returns a weight for a given document, and is used by some
weighting schemes to account for influence such as document length.

Parameters:
-----------

len:  the (unnormalised) document length. ";

%feature("docstring")  Xapian::Weight::get_maxextra "

Gets the maximum value that get_sumextra() may return.

virtual Xapian::weight Xapian::Weight::get_maxextra() const =0

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

The monotonic counter used for automatically allocating document IDs
is increased so that the next automatically allocated document ID will
be did + 1. Be aware that if you use this method to specify a high
document ID for a new document, and also use
WritableDatabase::add_document(), Xapian may get to a state where this
counter wraps around and will be unable to automatically allocate
document IDs!

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

%feature("docstring")  Xapian::WritableDatabase::add_spelling "

Add a word to the spelling dictionary.

void Xapian::WritableDatabase::add_spelling(const std::string &word,
Xapian::termcount freqinc=1) const

If the word is already present, its frequency is increased.

Parameters:
-----------

word:  The word to add.

freqinc:  How much to increase its frequency by (default 1). ";

%feature("docstring")  Xapian::WritableDatabase::remove_spelling "

Remove a word from the spelling dictionary.

void Xapian::WritableDatabase::remove_spelling(const std::string
&word, Xapian::termcount freqdec=1) const

The word's frequency is decreased, and if would become zero or less
then the word is removed completely.

Parameters:
-----------

word:  The word to remove.

freqdec:  How much to decrease its frequency by (default 1). ";

%feature("docstring")  Xapian::WritableDatabase::add_synonym "

Add a synonym for a term.

void Xapian::WritableDatabase::add_synonym(const std::string &term,
const std::string &synonym) const

If synonym is already a synonym for term, then no action is taken. ";

%feature("docstring")  Xapian::WritableDatabase::remove_synonym "

Remove a synonym for a term.

void Xapian::WritableDatabase::remove_synonym(const std::string &term,
const std::string &synonym) const

If synonym isn't a synonym for term, then no action is taken. ";

%feature("docstring")  Xapian::WritableDatabase::clear_synonyms "

Remove all synonyms for a term.

void Xapian::WritableDatabase::clear_synonyms(const std::string &term)
const

If term has no synonyms, no action is taken. ";

%feature("docstring")  Xapian::WritableDatabase::set_metadata "

Set the user-specified metadata associated with a given key.

void Xapian::WritableDatabase::set_metadata(const std::string &key,
const std::string &value)

This method sets the metadata value associated with a given key. If
there is already a metadata value stored in the database with the same
key, the old value is replaced. If you want to delete an existing item
of metadata, just set its value to the empty string.

User-specified metadata allows you to store arbitrary information in
the form of (key,tag) pairs.

There's no hard limit on the number of metadata items, or the size of
the metadata values. Metadata keys have a limited length, which
depends on the backend. We recommend limiting them to 200 bytes. Empty
keys are not valid, and specifying one will cause an exception.

Metadata modifications are committed to disk in the same way as
modifications to the documents in the database are: i.e.,
modifications are atomic, and won't be committed to disk immediately
(see flush() for more details). This allows metadata to be used to
link databases with versioned external resources by storing the
appropriate version number in a metadata item.

You can also use the metadata to store arbitrary extra information
associated with terms, documents, or postings by encoding the termname
and/or document id into the metadata key.

Parameters:
-----------

key:  The key of the metadata item to set.

value:  The value of the metadata item to set.

Parameters:
-----------

Xapian::DatabaseError:  will be thrown if a problem occurs while
writing to the database.

Xapian::DatabaseCorruptError:  will be thrown if the database is in a
corrupt state.

Xapian::InvalidArgumentError:  will be thrown if the key supplied is
empty. ";

%feature("docstring")  Xapian::WritableDatabase::get_description "

Return a string describing this object.

std::string Xapian::WritableDatabase::get_description() const ";


// File: namespaceXapian.xml
%feature("docstring")  Xapian::Auto::sortable_serialise "

Convert a floating point number to a string, preserving sort order.

XAPIAN_VISIBILITY_DEFAULT std::string
Xapian::sortable_serialise(double value)

This method converts a floating point number to a string, suitable for
using as a value for numeric range restriction, or for use as a sort
key.

The conversion is platform independent.

The conversion attempts to ensure that, for any pair of values
supplied to the conversion algorithm, the result of comparing the
original values (with a numeric comparison operator) will be the same
as the result of comparing the resulting values (with a string
comparison operator). On platforms which represent doubles with the
precisions specified by IEEE_754, this will be the case: if the
representation of doubles is more precise, it is possible that two
very close doubles will be mapped to the same string, so will compare
equal.

Note also that both zero and -zero will be converted to the same
representation: since these compare equal, this satisfies the
comparison constraint, but it's worth knowing this if you wish to use
the encoding in some situation where this distinction matters.

Handling of NaN isn't (currently) guaranteed to be sensible. ";

%feature("docstring")  Xapian::Auto::sortable_unserialise "

Convert a string encoded using sortable_serialise back to a floating
point number.

XAPIAN_VISIBILITY_DEFAULT double Xapian::sortable_unserialise(const
std::string &value)

This expects the input to be a string produced by
sortable_serialise(). If the input is not such a string, the value
returned is undefined (but no error will be thrown).

The result of the conversion will be exactly the value which was
supplied to  sortable_serialise() when making the string on platforms
which represent doubles with the precisions specified by IEEE_754, but
may be a different (nearby) value on other platforms. ";

%feature("docstring")  Xapian::Auto::version_string "

Report the version string of the library which the program is linked
with.

XAPIAN_VISIBILITY_DEFAULT const char* Xapian::version_string()

This may be different to the version compiled against (given by
XAPIAN_VERSION) if shared libraries are being used. ";

%feature("docstring")  Xapian::Auto::major_version "

Report the major version of the library which the program is linked
to.

XAPIAN_VISIBILITY_DEFAULT int Xapian::major_version()

This may be different to the version compiled against (given by
XAPIAN_MAJOR_VERSION) if shared libraries are being used. ";

%feature("docstring")  Xapian::Auto::minor_version "

Report the minor version of the library which the program is linked
to.

XAPIAN_VISIBILITY_DEFAULT int Xapian::minor_version()

This may be different to the version compiled against (given by
XAPIAN_MINOR_VERSION) if shared libraries are being used. ";

%feature("docstring")  Xapian::Auto::revision "

Report the revision of the library which the program is linked to.

XAPIAN_VISIBILITY_DEFAULT int Xapian::revision()

This may be different to the version compiled against (given by
XAPIAN_REVISION) if shared libraries are being used. ";


// File: namespaceXapian_1_1Auto.xml
%feature("docstring")  Xapian::Auto::open_stub "

Construct a Database object for a stub database file.

XAPIAN_VISIBILITY_DEFAULT Database Xapian::Auto::open_stub(const
std::string &file)

The stub database file contains serialised parameters for one or more
databases.

Parameters:
-----------

file:  pathname of the stub database file. ";


// File: namespaceXapian_1_1Chert.xml
%feature("docstring")  Xapian::Chert::open "

Construct a Database object for read-only access to a Chert database.

XAPIAN_VISIBILITY_DEFAULT Database Xapian::Chert::open(const
std::string &dir)

Parameters:
-----------

dir:  pathname of the directory containing the database. ";

%feature("docstring")  Xapian::Chert::open "

Construct a Database object for update access to a Chert database.

XAPIAN_VISIBILITY_DEFAULT WritableDatabase Xapian::Chert::open(const
std::string &dir, int action, int block_size=8192)

Parameters:
-----------

dir:  pathname of the directory containing the database.

action:  determines handling of existing/non-existing database:
Xapian::DB_CREATE fail if database already exist, otherwise create new
database.

Xapian::DB_CREATE_OR_OPEN open existing database, or create new
database if none exists.

Xapian::DB_CREATE_OR_OVERWRITE overwrite existing database, or create
new database if none exists.

Xapian::DB_OPEN open existing database, failing if none exists.

block_size:  the Btree blocksize to use (in bytes), which must be a
power of two between 2048 and 65536 (inclusive). The default (also
used if an invalid value if passed) is 8192 bytes. This parameter is
ignored when opening an existing database. ";


// File: namespaceXapian_1_1Flint.xml
%feature("docstring")  Xapian::Flint::open "

Construct a Database object for read-only access to a Flint database.

XAPIAN_VISIBILITY_DEFAULT Database Xapian::Flint::open(const
std::string &dir)

Parameters:
-----------

dir:  pathname of the directory containing the database. ";

%feature("docstring")  Xapian::Flint::open "

Construct a Database object for update access to a Flint database.

XAPIAN_VISIBILITY_DEFAULT WritableDatabase Xapian::Flint::open(const
std::string &dir, int action, int block_size=8192)

Parameters:
-----------

dir:  pathname of the directory containing the database.

action:  determines handling of existing/non-existing database:
Xapian::DB_CREATE fail if database already exist, otherwise create new
database.

Xapian::DB_CREATE_OR_OPEN open existing database, or create new
database if none exists.

Xapian::DB_CREATE_OR_OVERWRITE overwrite existing database, or create
new database if none exists.

Xapian::DB_OPEN open existing database, failing if none exists.

block_size:  the Btree blocksize to use (in bytes), which must be a
power of two between 2048 and 65536 (inclusive). The default (also
used if an invalid value if passed) is 8192 bytes. This parameter is
ignored when opening an existing database. ";


// File: namespaceXapian_1_1InMemory.xml
%feature("docstring")  Xapian::InMemory::open "

Construct a Database object for update access to an InMemory database.

XAPIAN_VISIBILITY_DEFAULT WritableDatabase Xapian::InMemory::open()

A new, empty database is created for each call. ";


// File: namespaceXapian_1_1Internal.xml


// File: namespaceXapian_1_1Remote.xml
%feature("docstring")  Xapian::Remote::open "

Construct a Database object for read-only access to a remote database
accessed via a TCP connection.

XAPIAN_VISIBILITY_DEFAULT Database Xapian::Remote::open(const
std::string &host, unsigned int port, Xapian::timeout timeout=10000,
Xapian::timeout connect_timeout=10000)

Access to the remote database is via a TCP connection to the specified
host and port.

Parameters:
-----------

host:  hostname to connect to.

port:  port number to connect to.

timeout:  timeout in milliseconds. If this timeout is exceeded for any
individual operation on the remote database then
Xapian::NetworkTimeoutError is thrown. A timeout of 0 means don't
timeout. (Default is 10000ms, which is 10 seconds).

connect_timeout:  timeout to use when connecting to the server. If
this timeout is exceeded then Xapian::NetworkTimeoutError is thrown. A
timeout of 0 means don't timeout. (Default is 10000ms, which is 10
seconds). ";

%feature("docstring")  Xapian::Remote::open_writable "

Construct a WritableDatabase object for update access to a remote
database accessed via a TCP connection.

XAPIAN_VISIBILITY_DEFAULT WritableDatabase
Xapian::Remote::open_writable(const std::string &host, unsigned int
port, Xapian::timeout timeout=0, Xapian::timeout
connect_timeout=10000)

Access to the remote database is via a TCP connection to the specified
host and port.

Parameters:
-----------

host:  hostname to connect to.

port:  port number to connect to.

timeout:  timeout in milliseconds. If this timeout is exceeded for any
individual operation on the remote database then
Xapian::NetworkTimeoutError is thrown. (Default is 0, which means
don't timeout).

connect_timeout:  timeout to use when connecting to the server. If
this timeout is exceeded then Xapian::NetworkTimeoutError is thrown. A
timeout of 0 means don't timeout. (Default is 10000ms, which is 10
seconds). ";

%feature("docstring")  Xapian::Remote::open "

Construct a Database object for read-only access to a remote database
accessed via a program.

XAPIAN_VISIBILITY_DEFAULT Database Xapian::Remote::open(const
std::string &program, const std::string &args, Xapian::timeout
timeout=10000)

Access to the remote database is done by running an external program
and communicating with it on stdin/stdout.

Parameters:
-----------

program:  the external program to run.

args:  space-separated list of arguments to pass to program.

timeout:  timeout in milliseconds. If this timeout is exceeded for any
individual operation on the remote database then
Xapian::NetworkTimeoutError is thrown. A timeout of 0 means don't
timeout. (Default is 10000ms, which is 10 seconds). ";

%feature("docstring")  Xapian::Remote::open_writable "

Construct a WritableDatabase object for update access to a remote
database accessed via a program.

XAPIAN_VISIBILITY_DEFAULT WritableDatabase
Xapian::Remote::open_writable(const std::string &program, const
std::string &args, Xapian::timeout timeout=0)

Access to the remote database is done by running an external program
and communicating with it on stdin/stdout.

Parameters:
-----------

program:  the external program to run.

args:  space-separated list of arguments to pass to program.

timeout:  timeout in milliseconds. If this timeout is exceeded for any
individual operation on the remote database then
Xapian::NetworkTimeoutError is thrown. (Default is 0, which means
don't timeout). ";


// File: namespaceXapian_1_1Unicode.xml
%feature("docstring")  Xapian::Unicode::Internal::nonascii_to_utf8 "

Convert a single non-ASCII unicode character to UTF-8.

XAPIAN_VISIBILITY_DEFAULT unsigned
Xapian::Unicode::nonascii_to_utf8(unsigned ch, char *buf)

This is intended mainly as a helper method for to_utf8().

The character ch (which must be > 128) is written to the buffer buf
and the length of the resultant UTF-8 character is returned.

NB buf must have space for (at least) 4 bytes. ";

%feature("docstring")  Xapian::Unicode::Internal::to_utf8 "

Convert a single unicode character to UTF-8.

unsigned Xapian::Unicode::to_utf8(unsigned ch, char *buf)

The character ch is written to the buffer buf and the length of the
resultant UTF-8 character is returned.

NB buf must have space for (at least) 4 bytes. ";

%feature("docstring")  Xapian::Unicode::Internal::append_utf8 "

Append the UTF-8 representation of a single unicode character to a
std::string.

void Xapian::Unicode::append_utf8(std::string &s, unsigned ch) ";

%feature("docstring")  Xapian::Unicode::Internal::get_category "

Return the category which a given unicode character falls into.

category Xapian::Unicode::get_category(unsigned ch) ";

%feature("docstring")  Xapian::Unicode::Internal::is_wordchar "

Test is a given unicode character is a letter or number.

bool Xapian::Unicode::is_wordchar(unsigned ch) ";

%feature("docstring")  Xapian::Unicode::Internal::is_whitespace "

Test is a given unicode character is a whitespace character.

bool Xapian::Unicode::is_whitespace(unsigned ch) ";

%feature("docstring")  Xapian::Unicode::Internal::is_currency "

Test is a given unicode character is a currency symbol.

bool Xapian::Unicode::is_currency(unsigned ch) ";

%feature("docstring")  Xapian::Unicode::Internal::tolower "

Convert a unicode character to lowercase.

unsigned Xapian::Unicode::tolower(unsigned ch) ";

%feature("docstring")  Xapian::Unicode::Internal::toupper "

Convert a unicode character to uppercase.

unsigned Xapian::Unicode::toupper(unsigned ch) ";

%feature("docstring")  Xapian::Unicode::Internal::tolower "

Convert a UTF-8 std::string to lowercase.

std::string Xapian::Unicode::tolower(const std::string &term) ";

%feature("docstring")  Xapian::Unicode::Internal::toupper "

Convert a UTF-8 std::string to uppercase.

std::string Xapian::Unicode::toupper(const std::string &term) ";


// File: namespaceXapian_1_1Unicode_1_1Internal.xml
%feature("docstring")  Xapian::Unicode::Internal::get_character_info "XAPIAN_VISIBILITY_DEFAULT int
Xapian::Unicode::Internal::get_character_info(unsigned ch) ";

%feature("docstring")  Xapian::Unicode::Internal::get_case_type "

Extract how to convert the case of a unicode character from its info.

int Xapian::Unicode::Internal::get_case_type(int info) ";

%feature("docstring")  Xapian::Unicode::Internal::get_category "

Extract the category of a unicode character from its info.

category Xapian::Unicode::Internal::get_category(int info) ";

%feature("docstring")  Xapian::Unicode::Internal::get_delta "

Extract the delta to use for case conversion of a character from its
info.

int Xapian::Unicode::Internal::get_delta(int info) ";


// File: xapian_8h.xml


// File: base_8h.xml


// File: database_8h.xml


// File: dbfactory_8h.xml


// File: deprecated_8h.xml


// File: document_8h.xml


// File: enquire_8h.xml


// File: errorhandler_8h.xml


// File: expanddecider_8h.xml


// File: matchspy_8h.xml


// File: positioniterator_8h.xml


// File: postingiterator_8h.xml


// File: postingsource_8h.xml


// File: query_8h.xml


// File: queryparser_8h.xml


// File: replication_8h.xml


// File: sorter_8h.xml


// File: stem_8h.xml


// File: termgenerator_8h.xml


// File: termiterator_8h.xml


// File: types_8h.xml


// File: unicode_8h.xml


// File: valueiterator_8h.xml


// File: valuesetmatchdecider_8h.xml


// File: visibility_8h.xml


// File: dir_8311af467e29f3ec02f9ddc6e3a07394.xml


// File: dir_0d07fe9acb0f4c634dfecfcad5a6d6f3.xml

