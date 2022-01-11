==============
Omega overview
==============

If you just want a very quick overview, you might prefer to read the
`quick-start guide <quickstart.html>`_.

Omega operates on a set of databases.  Each database is created and updated
separately using either omindex or `scriptindex <scriptindex.html>`_.  You can
search these databases (or any other Xapian database with suitable contents)
via a web front-end provided by omega, a CGI application.  A search can also be
done over more than one database at once.

There are separate documents covering `CGI parameters <cgiparams.html>`_, the
`Term Prefixes <termprefixes.html>`_ which are conventionally used, and
`OmegaScript <omegascript.html>`_, the language used to define omega's web
interface.  Omega ships with several OmegaScript templates and you can
use these, modify them, or just write your own.  See the "Supplied Templates"
section below for details of the supplied templates.

Omega parses queries using the ``Xapian::QueryParser`` class - for the supported
syntax, see queryparser.html in the xapian-core documentation
- available online at: https://xapian.org/docs/queryparser.html

Term construction
=================

Documents within an omega database are indexed by two types of terms: those
used for a weighted search from a parsed query string (the CGI parameter
``P``), and those used for boolean filtering (the CGI parameters ``B`` and
``N`` - the latter is a negated variant of 'B' and was added in Omega 1.3.5).

Boolean terms always start with a prefix which is an initial capital letter (or
multiple capital letters if the first character is `X`) which denotes the
category of the term (e.g. `M` for MIME type).

Parsed query terms may have a prefix, but don't always.  Those from the body of
the document in unstemmed form don't; stemmed terms have a `Z` prefix; terms
from other fields have a prefix to indicate the field, such as `S` for the
document title; stemmed terms from a field have both prefixes, e.g. `ZS`.

The "english" stemmer is used by default - you can configure this for omindex
and scriptindex with ``--stemmer=LANGUAGE`` (use ``--stemmer=none`` to disable
stemming, see omindex ``--help`` for the list of accepted language names).  At
search time you can configure the stemmer by adding ``$set{stemmer,LANGUAGE}``
to the top of your OmegaScript template.

The two term types are used as follows when building the query:

The ``P`` parameter is parsed using `Xapian::QueryParser` to give a
`Xapian::Query` object denoted as `P-terms` below.

There are two ways that ``B`` and ``N`` parameters are handled, depending if
the term-prefix has been configured as "non-exclusive" or not.  The default is
"exclusive" (and in versions before 1.3.4, this was how all ``B`` parameters
were handled).

Exclusive Boolean Prefix
------------------------

B(oolean) terms from 'B' parameters with the same prefix are ORed together,
like so::


                    [   OR   ]
                   /    | ... \
              B(F,1) B(F,2)...B(F,n)

Where B(F,1) is the first boolean term with prefix F from a 'B' parameter, and
so on.

Non-Exclusive Boolean Prefix
----------------------------

For example, ``$setmap{nonexclusiveprefix,K,true}`` sets prefix `K` as
non-exclusive, which means that multiple filter terms from 'B' parameters will
be combined with "AND" instead of "OR", like so::

                    [   AND   ]
                   /     | ... \
              B(K,1) B(K,2)... B(K,m)

Combining the Boolean Filters
-----------------------------

The subqueries for each prefix from "B" parameters are combined with AND,
to make this (which we refer to as "B-filter" below)::

                         [     AND     ]
                        /       |  ...  \
                       /                 \
                 [   OR   ]               [   AND  ]
                /    | ... \             /    | ... \
           B(F,1) B(F,2)...B(F,n)   B(K,1) B(K,2)...B(K,m)


Negated Boolean Terms
---------------------

All the terms from all 'N' parameters are combined together with "OR", to
make this (which we refer to as "N-filter" below)::

                    [       OR       ]
                   / ... |     |  ... \
              N(F,1)...N(F,n) N(K,1)...N(K,m)

Putting it all together
-----------------------

The P-terms are filtered by the B-filter using "FILTER" and by the N-filter
using "AND_NOT"::

                        [ AND_NOT ]
                       /           \
                      /             \
            [ FILTER ]             N-terms
             /      \
            /        \
       P-terms      B-terms

The intent here is to allow filtering on arbitrary (and, typically,
orthogonal) characteristics of the document. For instance, by adding
boolean terms "Ttext/html", "Ttext/plain" and "J/press" you would be
filtering the parsed query to only retrieve documents that are both in
the "/press" site *and* which are either of MIME type text/html or
text/plain. (See below for more information about sites.)

If B-terms or N-terms is absent, that part of the query is simply omitted.

If there is no parsed query, the boolean filter is promoted to
be the query, and the weighting scheme is set to boolean.  This has
the effect of applying the boolean filter to the whole database.  If
there are only N-terms, then ``Query::MatchAll`` is used for the left
side of the "AND_NOT".

In order to add more boolean prefixes, you will need to alter the
``index_file()`` function in omindex.cc. Currently omindex adds several
useful ones, detailed below.

Parsed query terms are constructed from the title, body and keywords
of a document. (Not all document types support all three areas of
text.) Title terms are stored with position data starting at 0, body
terms starting 100 beyond title terms, and keyword terms starting 100
beyond body terms. This allows queries using positional data without
causing false matches across the different types of term.

Sites
=====

Within a database, Omega supports multiple sites. These are recorded
using boolean terms (see 'Term construction', above) to allow
filtering on them.

Sites work by having all documents within them having a common base
URL. For instance, you might have two sites, one for your press area
and one for your product descriptions:

- \http://example.com/press/index.html
- \http://example.com/press/bigrelease.html
- \http://example.com/products/bigproduct.html
- \http://example.com/products/littleproduct.html

You could index all documents within \http://example.com/press/ using a
site of '/press', and all within \http://example.com/products/ using
'/products'.

Sites are also useful because omindex indexes documents through the
file system, not by fetching from the web server. If you don't have a
URL to file system mapping which puts all documents under one
hierarchy, you'll need to index each separate section as a site.

An obvious example of this is the way that many web servers map URLs
of the form <\http://example.com/~<username>/> to a directory within
that user's home directory (such as ~<username>/pub on a Unix
system). In this case, you can index each user's home page separately,
as a site of the form '/~<username>'. You can then use boolean
filters to allow people to search only a specific home page (or a
group of them), or omit such terms to search everyone's pages.

Note that the site specified when you index is used to build the
complete URL that the results page links to. Thus while sites will
typically want to be relative to the hostname part of the URL (e.g.
'/site' rather than '\http://example.com/site'), you can use them
to have a single search across several different hostnames. This will
still work if you actually store each distinct hostname in a different
database.

omindex operation
=================

omindex is fairly simple to use, for example::

  omindex --db default --url http://example.com/ /var/www/example.com

For a full list of command line options supported, see ``man omindex``
or ``omindex --help``.

You *must* specify the database to index into (it's created if it doesn't
exist, but parent directories must exist).  You will often also want to specify
the base URL (which is used as the site, and can be relative to the hostname -
starts '/' - or absolute - starts with a scheme, e.g.
'\http://example.com/products/').  If not specified, the base URL defaults to
``/``.

You also need to tell omindex which directory to index. This should be
either a single directory (in which case it is taken to be the
directory base of the entire site being indexed), or as two arguments,
the first being the directory base of the site being indexed, and the
second being a relative directory within that to index.

For instance, in the example above, if you separate your products by
size, you might end up with:

- \http://example.com/press/index.html
- \http://example.com/press/bigrelease.html
- \http://example.com/products/large/bigproduct.html
- \http://example.com/products/small/littleproduct.html

If the entire website is stored in the file system under the directory
/www/example, then you would probably index the site in two
passes, one for the '/press' site and one for the '/products' site. You
might use the following commands::

$ omindex -p --db /var/lib/omega/data/default --url /press /www/example/press
$ omindex -p --db /var/lib/omega/data/default --url /products /www/example/products

If you add a new large products, but don't want to reindex the whole of
the products section, you could do::

$ omindex -p --db /var/lib/omega/data/default --url /products /www/example/products large

and just the large products will be reindexed. You need to do it like that, and
not as::

$ omindex -p --db /var/lib/omega/data/default --url /products/large /www/example/products/large

because that would make the large products part of a new site,
'/products/large', which is unlikely to be what you want, as large
products would no longer come up in a search of the products
site. (Note that the ``--depth-limit`` option may come in handy if you have
sites '/products' and '/products/large', or similar.)

omindex has built-in support for indexing HTML, PHP, text files, CSV
(Comma-Separated Values) files, SVG, Atom feeds, and AbiWord documents.  It can
also index a number of other formats using external programs or libraries.  Filter programs and libraries
are run with CPU, time and memory limits to prevent them from
blocking indexing of other files or crashing omindex. If for one format both
options are available, libraries would be preferred because they have a better runtime behaviour.

The way omindex decides how to index a file is based around MIME content-types.
First of all omindex will look up a file's extension in its extension to MIME
type map.  If there's no entry, it will then ask libmagic to examine the
contents of the file and try to determine a MIME type.

The following formats are supported as standard (you can tell omindex to use
other filters too - see below):

* HTML (.html, .htm, .shtml, .shtm, .xhtml, .xhtm)
* PHP (.php) - our HTML parser knows to ignore PHP code
* text files (.txt, .text)
* SVG (.svg)
* CSV (Comma-Separated Values) files (.csv)
* PDF (.pdf) if pdftotext (comes with poppler or xpdf) or libpoppler
  (in particular libpoppler-glib-dev) are available
* PostScript (.ps, .eps, .ai) if ps2pdf (from ghostscript) and pdftotext (comes
  with poppler or xpdf) or libpoppler (in particular libpoppler-glib-dev) are available
* OpenOffice/StarOffice documents (.sxc, .stc, .sxd, .std, .sxi, .sti, .sxm,
  .sxw, .sxg, .stw) if unzip is available
* OpenDocument format documents (.odt, .ods, .odp, .odg, .odc, .odf, .odb,
  .odi, .odm, .ott, .ots, .otp, .otg, .otc, .otf, .oti, .oth) if unzip is
  available
* MS Word documents (.dot) if antiword is available (.doc files are left to
  libmagic, as they may actually be RTF (AbiWord saves RTF when asked to save
  as .doc, and Microsoft Word quietly loads RTF files with a .doc extension),
  or plain-text).
* MS Excel documents (.xls, .xlb, .xlt, .xlr, .xla) if xls2csv is available
  (comes with catdoc)
* MS Powerpoint documents (.ppt, .pps) if catppt is available (comes with
  catdoc)
* MS Office 2007 documents (.docx, .docm, .dotx, .dotm, .xlsx, .xlsm, .xltx,
  .xltm, .pptx, .pptm, .potx, .potm, .ppsx, .ppsm) if unzip is available
* Wordperfect documents (.wpd) if wpd2text is available (comes with libwpd)
* MS Works documents (.wps, .wpt) if wps2text is available (comes with libwps)
* MS Outlook message (.msg) if perl with Email::Outlook::Message and
  HTML::Parser modules is available
* MS Publisher documents (.pub) if pub2xhtml is available (comes with libmspub)
* MS Visio documents (.vsd, .vss, .vst, .vsw, .vsdx, .vssx, .vstx, .vsdm,
  .vssm, .vstm) if vsd2xhtml is available (comes with libvisio)
* Apple Keynote documents (.key, .kth, .apxl) if libetonyek is available (it is
  also possible to use key2text as an external filter)
* Apple Numbers documents (.numbers) if libetonyek is available (it is
  also possible to use numbers2text as an external filter)
* Apple Pages documents (.pages) if libetonyek is available (it is
  also possible to use pages2text as an external filter)
* AbiWord documents (.abw, .awt)
* Compressed AbiWord documents (.zabw)
* Rich Text Format documents (.rtf) if unrtf is available
* Perl POD documentation (.pl, .pm, .pod) if pod2text is available
* reStructured text (.rst, .rest) if rst2html is available (comes with
  docutils)
* Markdown (.md, .markdown) if markdown is available
* TeX DVI files (.dvi) if catdvi is available
* DjVu files (.djv, .djvu) if djvutxt is available
* OpenXPS and XPS files (.oxps, .xps) if unzip is available
* Debian packages (.deb, .udeb) if dpkg-deb is available
* RPM packages (.rpm) if rpm is available
* Atom feeds (.atom)
* MAFF (.maff) if unzip is available
* MHTML (.mhtml, .mht) if perl with MIME::Tools is available
* MIME email messages (.eml) and USENET articles if gmime 2.6 or perl with MIME::Tools and
  HTML::Parser is available
* vCard files (.vcf, .vcard) if perl with Text::vCard is available
* FictionBook v.2 files (.fb2) if libe-book is available
* QiOO (mobile format, for java-enabled cellphones) files (.jar) if libe-book is available
* TCR (simple compressed text format) files (.tcr) if libe-book is available
* eReader files (.pdb) if libe-book is available
* Sony eBook files (.lrf) if libe-book is available
* Image files that contain text (.png, .jpg, .jpeg, .jfif, .jpe, .webp, .tif, .tiff,
  .pbm, .gif, .ppm, .pgm) if libtesseract-dev is available.

If you have additional extensions that represent one of these types, you can
add an additional MIME mapping using the ``--mime-type`` option.  For
instance, if your press releases are PostScript files with extension
``.posts`` you can tell omindex this like so::

$ omindex --db /var/lib/omega/data/default --url /press /www/example/press --mime-type posts:application/postscript

The syntax of ``--mime-type`` is 'ext:type', where ext is the extension of
a file of that type (everything after the last '.').  The ``type`` can be any
string, but to be useful there either needs to be a filter set for that type
- either using ``--filter`` or ``--read-filters``, or by ``type`` being
understood by default:

.. include:: inc/mimetypes.rst

You can specify ``*`` as the MIME sub-type for ``--filter``, for example if you
have a filter you want to apply to any video files, you could specify it using
``--filter 'video/*:index-video-file'``.  Note that this is checked right after
checking for the exact MIME type, so will override any built-in filters which
would otherwise match.  Also you can't use arbitrary wildcards, just ``*`` for
the entire sub-type.  And be careful to quote ``*`` to protect it from the
shell.  Support for this was added in 1.3.3.

If there's no specific filter, and no subtype wildcard, then ``*/*`` is checked
(assuming the mimetype contains a ``/``), and after that ``*`` (for any
mimetype string).  Combined with filter command ``true`` for indexing by
meta-data only, you can specify a fall back case of indexing by meta-data
only using ``--filter '*:true'``.  Support for this was added in 1.3.4.

There are also two special values that can be specified instead of a MIME
type:

* ignore - tells omindex to quietly ignore such files
* skip - tells omindex to skip such files

By default no extensions are marked as "skip", and the following extensions are
marked as "ignore":

.. include:: inc/ignored.rst

If you wish to remove a MIME mapping, you can do this by omitting the type -
for example if you have ``.dot`` files which are inputs for the graphviz
tool ``dot``, then you may wish to remove the default mapping for ``.dot``
files and let libmagic be used to determine their type, which you can do
using: ``--mime-type=dot:`` (if you want to *ignore* all ``.dot`` files,
instead use ``--mime-type=dot:ignore``).

The lookup of extensions in the MIME mappings is case sensitive, but if an
extension isn't found and includes upper case ASCII letters, they're converted
to lower case and the lookup is repeated, so you effectively get case
insensitive lookup for mappings specified with a lower-case extension, but
you can set different handling for differently cased variants if you need
to.

You can add support for additional MIME content types (or override existing
ones) using the ``--filter`` and/or ``--read-filters`` options to specify a
command to run.  At present, this command needs to produce output in either
HTML, SVG, or plain text format (as of 1.3.3, you can specify the character
encoding that the output will be in; in earlier versions, plain text output had
to be UTF-8).  Support for SVG output from external commands was added in
1.4.8.

If you need to use a literal ``%`` in the command string, it needs to be
written as ``%%`` (since 1.3.3).

This command can take input in the following ways:

* (Since 1.5.0): If the command string has a ``|`` prefix, then the input file
  will be fed to the command on ``stdin``.  This is slightly more efficient as
  it often avoids having to open the input file an extra time (omindex needs to
  open the input file so it can calculate a checksum of the contents for
  duplicate detection, and also may need to use libmagic to find the file's
  MIME Content-Type).  In the future it will probably also allow extracting
  text from documents attached to emails, in ZIP files, etc without having to
  write them to a temporary file to run the filter on them.

* (Since 1.3.3): Any ``%f`` placeholder in the command string will be replaced
  with the filename of the file to extract (suitably escaped to protect it from
  the shell, so don't put quotes around ``%f``).

* If neither are present (and always in versions before 1.3.3) the filename is
  appended to the command (suitably escaped to protect it from the shell).

Output from the command can be handled in the following ways:

* (Since 1.3.3): Any ``%t`` in this command will be replaced with a filename in
  a temporary directory (suitably escaped to protect it from the shell, so
  don't put quotes around ``%t``).  The extension of this filename will reflect
  the expected output format (either ``.html``, ``.svg`` or ``.txt``).

* If you don't use ``%t`` in the command, then omindex will expect output on
  ``stdout`` (prior to 1.3.3, output had to be on ``stdout``).

For example, if you'd prefer to use Abiword to extract text from word documents
(by default, omindex uses antiword), then you can pass the option
``--filter=application/msword:'abiword --to=txt --to-name=fd://1'`` to
omindex.

Another example - if you wanted to handle files of MIME type
``application/octet-stream`` by piping them into ``strings -n8``, you can
pass the option ``--filter=application/octet-stream:'|strings -n8'`` (since
``strings`` reads from ``stdin`` if no filename is specified, at least in
the GNU binutils implementation).

A more complex example: to process ``.foo`` files with the (fictional)
``foo2utf16`` utility which produces UTF-16 text but doesn't support writing
output to stdout, run omindex with ``-Mfoo:text/x-foo
-Ftext/x-foo,,utf-16:'foo2utf16 %f %t'``.

A less contrived example of the use of ``--filter`` makes use of LibreOffice,
via the unoconv script, to extract text from various formats.  First you
need to start a listening instance (if you don't, unoconv will start up
LibreOffice for every file, which is rather inefficient) - the ``&`` tells
the shell to run it in the background::

  unoconv --listener &

Then run omindex with options such as
``--filter=application/msword,html:'unoconv --stdout -f html'`` (you'll want
to repeat this for each format which you want to use LibreOffice on).

If you specify ``false`` as the command in ``--filter``, omindex will skip
files with the specified MIME type.  (As of 1.2.20 and 1.3.3 ``false`` is
explicitly checked for; in earlier versions this will also work, at least
on Unix where ``false`` is a command which ignores its arguments and exits with
a non-zero status).

If you specify ``true`` as the command in ``--filter``, omindex won't try
to extract text from the file, but will index it such that it can be searched
for via metadata which comes from the filing system (filename, extension, mime
content-type, last modified time, size).  (As of 1.2.22 and 1.3.4 ``true`` is
explicitly checked for; in earlier versions this will also work, at least
on Unix where ``true`` is a command which ignores its arguments and exits with
a status zero).

If you know of a reliable filter which can extract text from a file format
which might be of interest to others, please let us know so we can consider
including it as a standard filter.

The ``--duplicates`` option controls how omindex handles documents which map
to a URL which is already in the database.  The default (which can be
explicitly set with ``--duplicates=replace``) is to reindex if the last
modified time of the file is newer than that recorded in the database.
The alternative is ``--duplicates=ignore``, which will never reindex an
existing document.  If you only add documents, this avoids the overhead
of checking the last modified time.  It also allows you to prioritise
adding completely new documents to the database over updating existing ones.

By default, omindex will remove any document in the database which has a URL
that doesn't correspond to a file seen on disk - in other words, it will clear
out everything that doesn't exist any more.  However if you are building up
an omega database with several runs of omindex, this is not
appropriate (as each run would delete the data from the previous run),
so you should use the ``--no-delete`` option.  Note that if you
choose to work like this, it is impossible to prune old documents from
the database using omindex. If this is a problem for you, an
alternative is to index each subsite into a different database, and
merge all the databases together when searching.

``--depth-limit`` allows you to prevent omindex from descending more than
a certain number of directories.  Specifying ``--depth-limit=0`` means no limit
is imposed on recursion; ``--depth-limit=1`` means don't descend into any
subdirectories of the start directory.

Tracking files which couldn't be indexed
----------------------------------------

In older versions, omindex only tracked files which it successfully indexed -
if a file couldn't be read, or a filter program failed on it, or it was marked
not to be indexed (e.g. with an HTML meta tag) then it would be retried on
subsequent runs.  Starting from version 1.3.4, omindex now tracks failed
files in the user metadata of the database, along with their sizes and last
modified times, and uses this data to skip files which previously failed and
haven't changed since.

You can force omindex to retry such files using the ``--retry-failed`` option.
One situation in which this is useful is if you've upgraded a filter program
to a newer version which you suspect will index some files which previously
failed.

Currently there's no mechanism for automatically removing failure entries
when the file they refer to is removed or renamed.  These lingering entries are
harmless, except they bloat the database a little.  A simple way to clear them
out is to run periodically with ``--retry-failed`` as this removes any existing
failure entries before indexing starts.

HTML Parsing
============

The document ``<title>`` tag is used as the document title.  Metadata in various
``<meta>`` tags is also understood - these values of the ``name`` parameter are
currently handled when found:

 * ``author``, ``dcterms.creator``, ``dcterms.contributor``: author(s)
 * ``created``, ``dcterms.issued``: document creation date
 * ``classification``: document topic
 * ``keywords``, ``dcterms.subject``, ``dcterms.description``: indexed as extra
   document text (but not stored in the sample)
 * ``description``: by default, handled as ``keywords``, as of Omega 1.4.4.
   If ``omindex`` is run with ``--sample=description``, then this is used as
   the preferred source for the stored sample of document text (HTML documents
   with no ``description`` fall back to a sample from the body; if
   ``description`` occurs multiple times then second and subsequent are handled
   as ``keywords``).  In Omega 1.4.2 and earlier, ``--sample`` wasn't supported
   and the behaviour was as if ``--sample=description`` had been specified.  In
   Omega 1.4.3, ``--sample`` was added, but the default was
   ``--sample=description`` (contrary to the intended and documented behaviour)
   - you can use ``--sample=body`` with 1.4.3 and later to store a sample from
   the document body.

The HTML parser will look for the 'robots' META tag, and won't index pages
which are marked as ``noindex`` or ``none``, for example any of the following::

    <meta name="robots" content="noindex,nofollow">
    <meta name="robots" content="noindex">
    <meta name="robots" content="none">

The ``omindex`` option ``--ignore-exclusions`` disables this behaviour, so
the files with the above will be indexed anyway.

Sometimes it is useful to be able to exclude just part of a page from being
indexed (for example you may not want to index navigation links, or a footer
which appears on every page).  To allow this, the parser supports "magic"
comments to mark sections of the document to not index.  Two formats are
supported - htdig_noindex (used by ht://Dig) and UdmComment (used by
mnoGoSearch)::

    Index this bit <!--htdig_noindex-->but <b>not</b> this<!--/htdig_noindex-->

::

    <!--UdmComment--><div>Boring copyright notice</div><!--/UdmComment-->

Boolean terms
=============

omindex will create the following boolean terms when it indexes a
document:

E
    Extension of the file (e.g. `Epdf`) [since Omega 1.2.5]
T
    MIME type

J
    The base URL, omitting any trailing slash (so if the base URL was just
    `/`, the term is just `J`).  If the resulting term would be > 240
    bytes, it's hashed in the same way an `U` prefix terms are.  Mnemonic: the
    Jumping-off point. [since Omega 1.3.4]
H
    hostname of site (if supplied - this term won't exist if you index a
    site with base URL '/press', for instance).  Since Omega 1.3.4, if the
    resulting term would be > 240 bytes, it's hashed in the same way as `U`
    prefix terms are.
P
    path terms - one term for the directory which the document is in, and for
    each parent directories, with no trailing slashes [since Omega 1.3.4 -
    in earlier versions, there was just one `P` term for the path of site (i.e.
    the rest of the site base URL) - this will be amongst the terms Omega 1.3.4
    adds].  Since Omega 1.3.4, if the resulting term would be > 240 bytes, it's
    hashed in the same way as `U` prefix terms are.
U
    full URL of indexed document - if the resulting term would be > 240 bytes,
    a hashing scheme is used to avoid overflowing Xapian's term length limit.

If the ``--date-terms`` option is used, then the following additional boolean
terms are added to documents (prior to 1.5.0 these were always added with no
way to disable this):

D
    date (numeric format: YYYYMMDD)

    date can also have the magical form "latest" - a document indexed
    by the term Dlatest matches any date-range without an end date.
    You can index dynamic documents which are always up to date
    with Dlatest and they'll match as expected.  (If you use sort by date,
    you'll probably also want to set the value containing the timestamp to
    a "max" value so dynamic documents match a date in the far future).
M
    month (numeric format: YYYYMM)
Y
    year (four digits)

omega configuration
===================

Most of the omega CGI configuration is dynamic, by setting CGI
parameters. However some things must be configured using a
configuration file.  The configuration file is searched for in
various locations:

- Firstly, if the "OMEGA_CONFIG_FILE" environment variable is
  set, its value is used as the full path to a configuration file
  to read.
- Next (if the environment variable is not set, or the file pointed
  to is not present), the file "omega.conf" in the same directory as
  the Omega CGI is used.
- Next (if neither of the previous steps found a file), the file
  "${sysconfdir}/omega.conf" (e.g. /etc/omega.conf on Linux systems)
  is used.
- Finally, if no configuration file is found, default values are used.

The format of the file is very simple: a line per option, with the
option name followed by its value, separated by a whitespace.  Blank
lines are ignored.  If the first non-whitespace character on a line
is a '#', omega treats the line as a comment and ignores it.

The current options are:

- `database_dir`: the directory containing all the Omega databases
- `template_dir`: the directory containing the OmegaScript templates
- `log_dir`: the directory which the OmegaScript `$log` command writes log
  files to
- `cdb_dir`: the directory which the OmegaScript `$lookup` command
  looks for CDB files in

The default values (used if no configuration file is found) are::

 database_dir /var/lib/omega/data
 template_dir /var/lib/omega/templates
 log_dir /var/log/omega
 cdb_dir /var/lib/omega/cdb

Note that, with apache, environment variables may be set using mod_env, and
with apache 1.3.7 or later this may be used inside a .htaccess file.  This
makes it reasonably easy to share a single system installed copy of Omega
between multiple users.

Supplied Templates
==================

The OmegaScript templates supplied with Omega are:

* query - This is the default template, providing a typical Web search
  interface.
* topterms - This is just like query, but provides a "top terms" feature
  which suggests terms the user might want to add to their query to
  obtain better results.
* godmode - Allows you to inspect a database showing which terms index
  each document, and which documents are indexed by each term.
* opensearch - Provides results in OpenSearch format (for more details
  see http://www.opensearch.org/).
* xml - Provides results in a custom XML format.
* emptydocs - Shows a list of documents with zero length.  If CGI parameter
  TERM is set to a non-empty value, then only documents indexed by that given
  term are shown (e.g. TERM=Tapplication/pdf to show PDF files with no text);
  otherwise all zero length documents are shown.

There are also "helper fragments" used by the templates above:

* inc/anyalldropbox - Provides a choice of matching "any" or "all" terms
  by default as a drop down box.
* inc/anyallradio - Provides a choice of matching "any" or "all" terms
  by default as radio buttons.
* toptermsjs - Provides some JavaScript used by the topterms template.

Document data construction
==========================

This is only useful if you need to inject your own documents into the
database independently of omindex, such as if you are indexing
dynamically-generated documents that are served using a server-side
system such as PHP or ASP, but which you can determine the contents of
in some way, such as documents generated from reasonably static
database contents.

The document data field stores some summary information about the
document, in the following (sample) format::

 url=<baseurl>
 sample=<sample>
 caption=<title>
 type=<mimetype>

Further fields may be added (although omindex doesn't currently add any
others), and may be looked up from OmegaScript using the $field{}
command.

As of Omega 0.9.3, you can alternatively add something like this near the
start of your OmegaScript template::

$set{fieldnames,$split{caption sample url}}

Then you need only give the field values in the document data, which can
save a lot of space in a large database.  With the setting of fieldnames
above, the first line of document data can be accessed with $field{caption},
the second with $field{sample}, and the third with $field{url}.

Stopword List
=============

At search time, Omega uses a built-in list of stopwords, which are::

    a about an and are as at be by en for from how i in is it of on or that the
    this to was what when where which who why will with you your
