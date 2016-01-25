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

Documents within an omega database are stored with two types of terms:
those used for probabilistic searching (the CGI parameter 'P'), and
those used for boolean filtering (the CGI parameter 'B'). Boolean
terms start with an initial capital letter denoting the 'group' of the
term (e.g. 'M' for MIME type), while probabilistic terms are all
lower-case, and are also stemmed before adding to the
database.

The "english" stemmer is used by default - you can configure this for omindex
and scriptindex with ``--stemmer=LANGUAGE`` (use ``--stemmer=none`` to disable
stemming, see omindex ``--help`` for the list of accepted language names).  At
search time you can configure the stemmer by adding ``$set{stemmer,LANGUAGE}``
to the top of your OmegaScript template.

The two term types are used as follows when building the query:
B(oolean) terms with the same prefix are ORed together, with all the
different prefix groups being ANDed together. This is then FILTERed
against the P(robabilistic) terms. This will look something like::

                  [ FILTER ]
                   /      \
                  /        \
             P-terms      [   AND   ]
                           /   |...\
                          /
                    [   OR   ]
                   /    | ... \
              B(F,1) B(F,2)...B(F,n)

Where B(F,1) is the first boolean term with prefix F, and so on.

The intent here is to allow filtering on arbitrary (and, typically,
orthogonal) characteristics of the document. For instance, by adding
boolean terms "Ttext/html", "Ttext/plain" and "P/press" you would be
filtering the probabilistic search for only documents that are both in
the "/press" site *and* which are either of MIME type text/html or
text/plain. (See below for more information about sites.)

If there is no probabilistic query, the boolean filter is promoted to
be the query, and the weighting scheme is set to boolean.  This has
the effect of applying the boolean filter to the whole database.

In order to add more boolean prefixes, you will need to alter the
``index_file()`` function in omindex.cc. Currently omindex adds several
useful ones, detailed below.

Probabilistic terms are constructed from the title, body and keywords
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
also index a number of other formats using external programs.  Filter programs
are run with CPU, time and memory limits to prevent a runaway filter from
blocking indexing of other files.

The way omindex decides how to index a file is based around MIME content-types.
First of all omindex will look up a file's extension in its extension to MIME
type map.  If there's no entry, and omindex was built with libmagic support,
then it will then ask libmagic to examine the contents of the file and try to
determine a MIME type.

The following formats are supported as standard (you can tell omindex to use
other filters too - see below):

* HTML (.html, .htm, .shtml)
* PHP (.php) - our HTML parser knows to ignore PHP code
* text files (.txt, .text)
* SVG (.svg)
* CSV (Comma-Separated Values) files (.csv)
* PDF (.pdf) if pdftotext is available (comes with poppler or xpdf)
* PostScript (.ps, .eps, .ai) if ps2pdf (from ghostscript) and pdftotext (comes
  with poppler or xpdf) are available
* OpenOffice/StarOffice documents (.sxc, .stc, .sxd, .std, .sxi, .sti, .sxm,
  .sxw, .sxg, .stw) if unzip is available
* OpenDocument format documents (.odt, .ods, .odp, .odg, .odc, .odf, .odb,
  .odi, .odm, .ott, .ots, .otp, .otg, .otc, .otf, .oti, .oth) if unzip is
  available
* MS Word documents (.doc, .dot) if antiword is available
* MS Excel documents (.xls, .xlb, .xlt, .xlr) if xls2csv is available (comes
  with catdoc)
* MS Powerpoint documents (.ppt, .pps) if catppt is available (comes with
  catdoc)
* MS Office 2007 documents (.docx, .docm, .dotx, .dotm, .xlsx, .xlsm, .xltx,
  .xltm, .pptx, .pptm, .potx, .potm, .ppsx, .ppsm) if unzip is available
* Wordperfect documents (.wpd) if wpd2text is available (comes with libwpd)
* MS Works documents (.wps, .wpt) if wps2text is available (comes with libwps)
* MS Outlook message (.msg) if perl with Email::Outlook::Message and
  HTML::Parser modules is available
* AbiWord documents (.abw)
* Compressed AbiWord documents (.zabw) if gzip is available
* Rich Text Format documents (.rtf) if unrtf is available
* Perl POD documentation (.pl, .pm, .pod) if pod2text is available
* TeX DVI files (.dvi) if catdvi is available
* DjVu files (.djv, .djvu) if djvutxt is available
* XPS files (.xps) if unzip is available
* Debian packages (.deb, .udeb) if dpkg-deb is available
* RPM packages (.rpm) if rpm is available
* Atom feeds (.atom)

If you have additional extensions that represent one of these types, you can
add an additional MIME mapping using the ``--mime-type`` option.  For
instance::

$ omindex --db /var/lib/omega/data/default --url /press /www/example/press --mime-type doc:application/postscript

The syntax of ``--mime-type`` is 'ext:type', where ext is the extension of
a file of that type (everything after the last '.').  The ``type`` can be any
string, but to be useful there either needs to be a filter set for that type
- either using ``--filter`` or by ``type`` being understood by default:

   - text/csv
   - text/html
   - text/plain
   - text/rtf
   - text/x-perl
   - application/atom+xml
   - application/msword
   - application/pdf
   - application/postscript
   - application/vnd.ms-excel
   - application/vnd.ms-outlook
   - application/vnd.ms-powerpoint
   - application/vnd.ms-works
   - application/vnd.ms-xpsdocument
   - application/vnd.oasis.opendocument.text
   - application/vnd.oasis.opendocument.spreadsheet
   - application/vnd.oasis.opendocument.presentation
   - application/vnd.oasis.opendocument.graphics
   - application/vnd.oasis.opendocument.chart
   - application/vnd.oasis.opendocument.formula
   - application/vnd.oasis.opendocument.database
   - application/vnd.oasis.opendocument.image
   - application/vnd.oasis.opendocument.text-master
   - application/vnd.oasis.opendocument.text-template
   - application/vnd.oasis.opendocument.spreadsheet-template
   - application/vnd.oasis.opendocument.presentation-template
   - application/vnd.oasis.opendocument.graphics-template
   - application/vnd.oasis.opendocument.chart-template
   - application/vnd.oasis.opendocument.formula-template
   - application/vnd.oasis.opendocument.image-template
   - application/vnd.oasis.opendocument.text-web
   - application/vnd.openxmlformats-officedocument.wordprocessingml.document
   - application/vnd.openxmlformats-officedocument.wordprocessingml.template
   - application/vnd.openxmlformats-officedocument.spreadsheetml.sheet
   - application/vnd.openxmlformats-officedocument.spreadsheetml.template
   - application/vnd.openxmlformats-officedocument.presentationml.presentation
   - application/vnd.openxmlformats-officedocument.presentationml.slideshow
   - application/vnd.openxmlformats-officedocument.presentationml.template
   - application/vnd.sun.xml.calc
   - application/vnd.sun.xml.calc.template
   - application/vnd.sun.xml.draw
   - application/vnd.sun.xml.draw.template
   - application/vnd.sun.xml.impress
   - application/vnd.sun.xml.impress.template
   - application/vnd.sun.xml.math
   - application/vnd.sun.xml.writer
   - application/vnd.sun.xml.writer.global
   - application/vnd.sun.xml.writer.template
   - application/vnd.wordperfect
   - application/x-abiword
   - application/x-abiword-compressed
   - application/x-debian-package
   - application/x-dvi
   - application/x-redhat-package-manager
   - image/svg+xml
   - image/vnd.djvu
   - ignore (magic token to tell omindex to quietly ignore such files)

By default, files with the following extensions are marked as 'ignore'::

   - a
   - adm
   - bin
   - com
   - css
   - cur
   - dat
   - db
   - dll
   - dylib
   - exe
   - fon
   - ico
   - jar
   - js
   - lib
   - lnk
   - o
   - obj
   - pyc
   - pyd
   - pyo
   - so
   - sqlite
   - sqlite3
   - sqlite-journal
   - tmp
   - ttf

If you wish to remove a MIME mapping, you can do this by omitting the type -
for example to not index .doc files, use: ``--mime-type=doc:``

The lookup of extensions in the MIME mappings is case sensitive, but if an
extension isn't found and includes upper case ASCII letters, they're converted
to lower case and the lookup is repeated, so you effectively get case
insensitive lookup for mappings specified with a lower-case extension, but
you can set different handling for differently cased variants if you need
to.

You can add support for additional MIME content types (or override existing
ones) using the ``--filter`` option to specify a command to run.  In Omega
1.2.x, this command needs to produce output on stdout in UTF-8 text format
(1.3.x also supports commands which produce HTML output).

For example, if you'd prefer to use Abiword to extract text from word documents
(by default, omindex uses antiword), then you can pass the option
``--filter=application/msword:'abiword --to=txt --to-name=fd://1'`` to
omindex.  The filename of the file to be extracted will be appended to this
command, separated by a space.

Another example - if you wanted to handle files of MIME type
``application/octet-stream`` by running them through ``strings -n8``, you can
pass the option ``--filter=application/octet-stream:'strings -n8'``.

A more complex example of the use of ``--filter`` makes use of LibreOffice,
via the unoconv script, to extract text from various formats.  First you
need to start a listening instance (if you don't, unoconv will start up
LibreOffice for every file, which is rather inefficient) - the ``&`` tells
the shell to run it in the background::

  unoconv --listener &

Then run omindex with options such as
``--filter=application/msword:'unoconv --stdout -f text'`` (you'll want one
for each format which you want to extract text from with LibreOffice).

If you specify ``false`` as the command in ``--filter``, omindex will skip
files with the specified MIME type.  (As of 1.2.20 and 1.3.3 ``false`` is
explicitly checked for; in earlier versions this will also work, at least
on Unix where ``false`` is a command which ignores its arguments and exits with
a non-zero status).

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

HTML Parsing
============

The document ``<title>`` tag is used as the document title, the 'description'
META tag (if present) is used for the document snippet, and the 'keywords'
META tag (if present) is indexed as extra document text.

The HTML parser will look for the 'robots' META tag, and won't index pages
which are marked as ``noindex`` or ``none``, for example any of the following::

    <meta name="robots" content="noindex,nofollow">
    <meta name="robots" content="noindex">
    <meta name="robots" content="none">

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
H
    hostname of site (if supplied - this term won't exist if you index a
    site with base URL '/press', for instance)
P
    path of site (i.e. the rest of the site base URL)
U
    full URL of indexed document - if the resulting term would be > 240 bytes,
    a hashing scheme is used to avoid overflowing Xapian's term length limit.

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
