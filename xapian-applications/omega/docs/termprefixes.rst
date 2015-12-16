=============
Term Prefixes
=============

Xapian itself doesn't put any restrictions on the contents of a term, other
than that terms can't be empty, and there's an upper limit on the length
(which is backend dependent - flint and chert allow 245 bytes, except that
zero bytes count double in this length).

However, Omega and ``Xapian::QueryParser`` impose some rules to aid
interoperability and make it easier to write code that doesn't require
excessive configuring.  It's probably wise to follow these rules unless
you have a good reason not to.  Right now you might not intend to use Omega
or the QueryParser, not to combine a search with another database.  But if
you later find you do, it'll be much easier if you're using compatible
rules!

The basic idea is that terms won't begin with a capital letter (since they're
usually lower-cased and often stemmed), so any term which starts with a capital
letter is assumed to have a prefix.  For all letters apart from X, this is a
single character prefix and these have predefined standard meanings (or are
reserved for standard meanings but currently unallocated).

X starts a multi-capital letter user-defined prefix.  If you want a prefix for
something without a standard prefix, you create your own starting with an X
(e.g. XSHOESIZE).  The prefix ends with the first non-capital.  If the term
you're prefixing starts with a capital, add a ":" between prefix and term to
resolve ambiguity about where the prefix ends and the term begins.

Here's the current allocation list:

A	
        Author
D	
        Date (numeric format: YYYYMMDD or "latest" - e.g. D20050224 or Dlatest)
E
        Extension (folded to lowercase - e.g. Ehtml, or E for no extension)
G	
        newsGroup (or similar entity - e.g. a web forum name)
H	
        Hostname
I
	boolean filter term for "can see" permission (mnemonic: Include)
K	
        Keyword
L	
        ISO Language code
M	
        Month (numeric format: YYYYMM)
N	
        ISO couNtry code (or domaiN name)
O
	Owner
P	
        Pathname
Q	
        uniQue id
R	
        Raw (i.e. unstemmed) term (unused by Xapian since 1.0.0)
S	
        Subject (or title)
T	
        mimeType
U	
        full URL of indexed document - if the resulting term would be > 240
	bytes, a hashing scheme is used to prevent overflowing
	the Xapian term length limit (see omindex for how to do this).
V
	boolean filter term for "can't see" permission (mnemonic: grep -v)
X	
        longer prefix for user-defined use
Y	
        year (four digits)
Z	
        stemmed term

Reserved but currently unallocated: BCFJW

There are two main uses for prefixes - boolean filters and probabilistic
(i.e. free text) fields.

Boolean Filters
===============

If the documents being indexed represent people, you might have a gender
field (e.g. M for Male, F for Female, X for Unknown).  Gender doesn't have
a standard prefix, so you might allocated "XGENDER".  And then lowercase
the field contents to avoid needing to always add a colon.  So documents
will be indexed by one of XGENDERm, XGENDERf, or XGENDERx.

If you're indexing using scriptindex, and have a field in the input file
which can be "gender=M", etc, then your index script would have a rule
such as::

    gender : lower boolean=XGENDER

You can then restrict a search in Omega by passing a B parameter with one
of these as the value, e.g. B=XGENDERf

In your HTML search form, you can allow the user to select this using a set of
radio buttons::

    Gender:<br>
    <input type="radio" name="B" value=""> any<br>
    <input type="radio" name="B" value="XGENDERf"> female<br>
    <input type="radio" name="B" value="XGENDERm"> male<br>

If you want to have multiple sets of radio buttons for selecting different
boolean filters, you can make use of Omega's preprocessing of CGI parameter
names by calling them "B 1", "B 2", etc (names are truncated at the first
space - see `cgiparams.html <cgiparams.html>`_ for full details).

You can also use a select tag::

    Gender:
    <select name="B">
    <option value="">any</option>
    <option value="XGENDERf">female</option>
    <option value="XGENDERm">male</option>
    <option value="XGENDERx">unknown</option>
    </select>

You can also allow the user to restrict a search with a boolean filter
specified in text query (e.g. sex:f -> XGENDERf) by adding this to the
start of your OmegaScript template::

    $setmap{boolprefix,sex,XGENDER}

Multiple aliases are allowed::

    $setmap{boolprefix,sex,XGENDER,gender,XGENDER}

This decoupling of internal and external names is also useful if you want
to offer search frontends in more than one language, as it allows the
prefixes the user sees to be translated.

Probabilistic Fields
====================

Say you want to index the title of the document such that the user can
search within the title by specifying title:report (for example) in their
query.

Title has standard prefix S, so you'd generate terms as normal, but then
add an "S" prefix.  If you're using scriptindex, then you do this by
adding "index=S" to the scriptindex rule like so::

    title : field=title index=S

You then need to tell Xapian::QueryParser that "title:" maps to an "S" prefix.
If you're using Omega, then you do so by adding this to your OmegaScript
template (at the start is best)::

    $setmap{prefix,title,S}

Or if you're writing your own search frontend, like this::

    Xapian::QueryParser qp;
    qp.add_prefix("subject", "S");
    // And similar lines for other probabilistic prefixes...
    // And any other QueryParser configuration (e.g. stemmer, stopper).
    Xapian::Query query = qp.parse_query(user_query_string);

You can add multiple aliases for a prefix (e.g. title and subject for S), and
the decoupling of "UI prefix" and "term prefix" means you can easily translate
the "UI prefixes" if you have frontends in different languages.

Note that if you want words from the subject to be found without a prefix, you
either need to generate unprefixed terms as well as the prefixed ones, or map
the empty prefix to both "" and "S" like so::

    Xapian::QueryParser qp;
    // Search both subject and body if no field is specified:
    qp.add_prefix("", "");
    qp.add_prefix("", "S");
    // Search just the subject if 'subject:' is specified:
    qp.add_prefix("subject", "S");
    Xapian::Query query = qp.parse_query(user_query_string);
