/** @file index_file.cc
 * @brief Handle indexing a document from a file
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2005 James Aylett
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2012,2013,2014,2015 Olly Betts
 * Copyright 2009 Frank J Bruzzaniti
 * Copyright 2012 Mihai Bivol
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include "index_file.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <string>
#include <map>
#include <vector>

#include <sys/types.h>
#include "safeunistd.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "safefcntl.h"
#include "safeerrno.h"
#include <ctime>

#include <xapian.h>

#include "append_filename_arg.h"
#include "atomparse.h"
#include "diritor.h"
#include "failed.h"
#include "md5wrap.h"
#include "metaxmlparse.h"
#include "mimemap.h"
#include "msxmlparse.h"
#include "myhtmlparse.h"
#include "opendocparse.h"
#include "pkglibbindir.h"
#include "runfilter.h"
#include "sample.h"
#include "str.h"
#include "stringutils.h"
#include "svgparse.h"
#include "tmpdir.h"
#include "utf8convert.h"
#include "utils.h"
#include "values.h"
#include "xmlparse.h"
#include "xlsxparse.h"
#include "xpsxmlparse.h"

using namespace std;

static Xapian::WritableDatabase db;
static Xapian::TermGenerator indexer;

static Xapian::doccount old_docs_not_seen;
static Xapian::docid old_lastdocid;
static vector<bool> updated;

static bool verbose;
static bool retry_failed;
static bool use_ctime;
static dup_action_type dup_action;
static bool ignore_exclusions;

static time_t last_altered_max;
static size_t sample_size;
static size_t title_size;
static size_t max_ext_len;

static empty_body_type empty_body;

static string root;
static string site_term, host_term;

static Failed failed;

map<string, Filter> commands;

static void
mark_as_seen(Xapian::docid did)
{
    if (usual(did < updated.size() && !updated[did])) {
	updated[did] = true;
	--old_docs_not_seen;
    }
}

void
skip(const string & urlterm, const string & context, const string & msg,
     off_t size, time_t last_mod, unsigned flags)
{
    failed.add(urlterm, last_mod, size);

    if (!verbose || (flags & SKIP_SHOW_FILENAME)) {
	if (!verbose && (flags & SKIP_VERBOSE_ONLY)) return;
	cout << context << ": ";
    }

    cout << "Skipping - " << msg << endl;
}

static void
skip_cmd_failed(const string & urlterm, const string & context, const string & cmd,
		off_t size, time_t last_mod)
{
    skip(urlterm, context, "\"" + cmd + "\" failed", size, last_mod);
}

static void
skip_meta_tag(const string & urlterm, const string & context,
	      off_t size, time_t last_mod)
{
    skip(urlterm, context, "indexing disallowed by meta tag", size, last_mod);
}

static void
skip_unknown_mimetype(const string & urlterm, const string & context,
		      const string & mimetype, off_t size, time_t last_mod)
{
    skip(urlterm, context, "unknown MIME type '" + mimetype + "'", size, last_mod);
}

void
index_add_default_filters()
{
    index_command("application/msword", Filter("antiword -mUTF-8.txt", false));
    index_command("application/vnd.ms-excel",
		  Filter("xls2csv -c' ' -q0 -dutf-8", false));
    index_command("application/vnd.ms-powerpoint",
		  Filter("catppt -dutf-8", false));
    // Looking at the source of wpd2html and wpd2text I think both output
    // UTF-8, but it's hard to be sure without sample Unicode .wpd files
    // as they don't seem to be at all well documented.
    index_command("application/vnd.wordperfect", Filter("wpd2text", false));
    // wps2text produces UTF-8 output from the sample files I've tested.
    index_command("application/vnd.ms-works", Filter("wps2text", false));
    // Output is UTF-8 according to "man djvutxt".  Generally this seems to
    // be true, though some examples from djvu.org generate isolated byte
    // 0x95 in a context which suggests it might be intended to be a bullet
    // (as it is in CP1250).
    index_command("image/vnd.djvu", Filter("djvutxt", false));
    // The --text option unhelpfully converts all non-ASCII characters to "?"
    // so we use --html instead, which produces HTML entities.  The --nopict
    // option suppresses exporting picture files as pictNNNN.wmf in the current
    // directory.  Note that this option was ignored in some older versions,
    // but it was fixed in unrtf 0.20.4.
    index_command("text/rtf",
		  Filter("unrtf --nopict --html 2>/dev/null", "text/html",
			 false));
    index_command("text/x-rst", Filter("rst2html", "text/html", false));
    index_command("application/x-mspublisher",
		  Filter("pub2xhtml", "text/html", false));
    index_command("application/vnd.ms-outlook",
		  Filter(get_pkglibbindir() + "/outlookmsg2html", "text/html",
			 false));
    // pod2text's output character set doesn't seem to be documented, but from
    // inspecting the source it looks like it's probably iso-8859-1.
    index_command("text/x-perl",
		  Filter("pod2text", "text/plain", "iso-8859-1", false));
    // FIXME: -e0 means "UTF-8", but that results in "fi", "ff", "ffi", etc
    // appearing as single ligatures.  For European languages, it's actually
    // better to use -e2 (ISO-8859-1) and then convert, so let's do that for
    // now until we handle Unicode "compatibility decompositions".
    index_command("application/x-dvi",
		  Filter("catdvi -e2 -s", "text/plain", "iso-8859-1", false));
    // Simplistic - ought to look in index.rdf files for filename and character
    // set.
    index_command("application/x-maff",
		  Filter("unzip -p %f '*/*.*htm*'", "text/html", "iso-8859-1",
			 false));
    index_command("application/x-mimearchive",
		  Filter(get_pkglibbindir() + "/mhtml2html", "text/html",
			 false));
    index_command("message/news",
		  Filter(get_pkglibbindir() + "/rfc822tohtml", "text/html",
			 false));
    index_command("message/rfc822",
		  Filter(get_pkglibbindir() + "/rfc822tohtml", "text/html",
			 false));
}

void
index_init(const string & dbpath, const Xapian::Stem & stemmer,
	   const string & root_, const string & site_term_,
	   const string & host_term_,
	   empty_body_type empty_body_, dup_action_type dup_action_,
	   size_t sample_size_, size_t title_size_, size_t max_ext_len_,
	   bool overwrite, bool retry_failed_,
	   bool delete_removed_documents, bool verbose_, bool use_ctime_,
	   bool spelling, bool ignore_exclusions_)
{
    root = root_;
    site_term = site_term_;
    host_term = host_term_;
    empty_body = empty_body_;
    dup_action = dup_action_;
    sample_size = sample_size_;
    title_size = title_size_;
    max_ext_len = max_ext_len_;
    verbose = verbose_;
    use_ctime = use_ctime_;
    ignore_exclusions = ignore_exclusions_;

    if (!overwrite) {
	db = Xapian::WritableDatabase(dbpath, Xapian::DB_CREATE_OR_OPEN);
	old_docs_not_seen = db.get_doccount();
	old_lastdocid = db.get_lastdocid();
	if (delete_removed_documents) {
	    // + 1 so that old_lastdocid is a valid subscript.
	    updated.resize(old_lastdocid + 1);
	}
	try {
	    Xapian::valueno slot = use_ctime ? VALUE_CTIME : VALUE_LASTMOD;
	    string ubound = db.get_value_upper_bound(slot);
	    if (!ubound.empty())
		last_altered_max = binary_string_to_int(ubound);
	} catch (const Xapian::UnimplementedError &) {
	    numeric_limits<time_t> n;
	    last_altered_max = n.max();
	}
    } else {
	db = Xapian::WritableDatabase(dbpath, Xapian::DB_CREATE_OR_OVERWRITE);
    }

    if (spelling) {
	indexer.set_database(db);
	indexer.set_flags(indexer.FLAG_SPELLING);
    }
    indexer.set_stemmer(stemmer);

    runfilter_init();

    failed.init(db);

    if (overwrite) {
	// There are no failures to retry, so setting this flag doesn't
	// change the outcome, but does mean we avoid the overhead of
	// checking for a previous failure.
	retry_failed = true;
    } else if (retry_failed_) {
	failed.clear();
	retry_failed = true;
    } else {
	// If there are no existing failures, setting this flag doesn't
	// change the outcome, but does mean we avoid the overhead of
	// checking for a previous failure.
	retry_failed = failed.empty();
    }
}

static void
parse_pdfinfo_field(const char * p, const char * end, string & out, const char * field, size_t len)
{
    if (size_t(end - p) > len && memcmp(p, field, len) == 0) {
	p += len;
	while (p != end && *p == ' ')
	    ++p;
	if (p != end && (end[-1] != '\r' || --end != p))
	    out.assign(p, end - p);
    }
}

#define PARSE_PDFINFO_FIELD(P, END, OUT, FIELD) \
    parse_pdfinfo_field((P), (END), (OUT), FIELD":", CONST_STRLEN(FIELD) + 1)

static void
get_pdf_metainfo(const string & file, string &author, string &title,
		 string &keywords, string &topic)
{
    try {
	string cmd = "pdfinfo -enc UTF-8";
	append_filename_argument(cmd, file);
	string pdfinfo = stdout_to_string(cmd, false);

	const char * p = pdfinfo.data();
	const char * end = p + pdfinfo.size();
	while (p != end) {
	    const char * start = p;
	    p = static_cast<const char *>(memchr(p, '\n', end - p));
	    const char * eol;
	    if (p) {
		eol = p;
		++p;
	    } else {
		p = eol = end;
	    }
	    switch (*start) {
		case 'A':
		    PARSE_PDFINFO_FIELD(start, eol, author, "Author");
		    break;
		case 'K':
		    PARSE_PDFINFO_FIELD(start, eol, keywords, "Keywords");
		    break;
		case 'S':
		    PARSE_PDFINFO_FIELD(start, eol, topic, "Subject");
		    break;
		case 'T':
		    PARSE_PDFINFO_FIELD(start, eol, title, "Title");
		    break;
	    }
	}
    } catch (ReadError) {
	// It's probably best to index the document even if pdfinfo fails.
    }
}

static void
generate_sample_from_csv(const string & csv_data, string & sample)
{
    // Add 3 to allow for a 4 byte utf-8 sequence being appended when
    // output is sample_size - 1 bytes long.  Use csv_data.size() if smaller
    // since the user might reasonably set sample_size really high.
    sample.reserve(min(sample_size + 3, csv_data.size()));
    size_t last_word_end = 0;
    bool in_space = true;
    bool in_quotes = false;
    for (Xapian::Utf8Iterator i(csv_data); i != Xapian::Utf8Iterator(); ++i) {
	unsigned ch = *i;

	if (!in_quotes) {
	    // If not already in double quotes, '"' starts quoting and
	    // ',' starts a new field.
	    if (ch == '"') {
		in_quotes = true;
		continue;
	    }
	    if (ch == ',')
		ch = ' ';
	} else if (ch == '"') {
	    // In double quotes, '"' either ends double quotes, or
	    // if followed by another '"', means a literal '"'.
	    if (++i == Xapian::Utf8Iterator())
		break;
	    ch = *i;
	    if (ch != '"') {
		in_quotes = false;
		if (ch == ',')
		    ch = ' ';
	    }
	}

	if (ch <= ' ' || ch == 0xa0) {
	    // FIXME: if all the whitespace characters between two
	    // words are 0xa0 (non-breaking space) then perhaps we
	    // should output 0xa0.
	    if (in_space)
		continue;
	    last_word_end = sample.size();
	    sample += ' ';
	    in_space = true;
	} else {
	    Xapian::Unicode::append_utf8(sample, ch);
	    in_space = false;
	}

	if (sample.size() >= sample_size) {
	    // Need to truncate sample.
	    if (last_word_end <= sample_size / 2) {
		// Monster word!  We'll have to just split it.
		sample.replace(sample_size - 3, string::npos, "...", 3);
	    } else {
		sample.replace(last_word_end, string::npos, " ...", 4);
	    }
	    break;
	}
    }
}

static bool
index_check_existing(const string & urlterm, time_t last_altered,
		     Xapian::docid & did)
{
    switch (dup_action) {
	case DUP_SKIP: {
	    Xapian::PostingIterator p = db.postlist_begin(urlterm);
	    if (p != db.postlist_end(urlterm)) {
		if (verbose)
		    cout << "already indexed, not updating" << endl;
		did = *p;
		mark_as_seen(did);
		return true;
	    }
	    break;
	}
	case DUP_CHECK_LAZILY: {
	    // If last_altered > last_altered_max, we know for sure that the
	    // file is new or updated.
	    if (last_altered > last_altered_max) {
		return false;
	    }

	    Xapian::PostingIterator p = db.postlist_begin(urlterm);
	    if (p != db.postlist_end(urlterm)) {
		did = *p;
		Xapian::Document doc = db.get_document(did);
		Xapian::valueno slot = use_ctime ? VALUE_CTIME : VALUE_LASTMOD;
		string value = doc.get_value(slot);
		time_t old_last_altered = binary_string_to_int(value);
		if (last_altered <= old_last_altered) {
		    if (verbose)
			cout << "already indexed" << endl;
		    // The docid should be in updated - the only valid
		    // exception is if the URL was long and hashed to the
		    // same URL as an existing document indexed in the same
		    // batch.
		    mark_as_seen(did);
		    return true;
		}
	    }
	    break;
	}
    }
    return false;
}

void
index_add_document(const string & urlterm, time_t last_altered,
		   Xapian::docid did, const Xapian::Document & doc)
{
    if (dup_action != DUP_SKIP) {
	// If this document has already been indexed, update the existing
	// entry.
	if (did) {
	    // We already found out the document id above.
	    db.replace_document(did, doc);
	} else if (last_altered <= last_altered_max) {
	    // We checked for the UID term and didn't find it.
	    did = db.add_document(doc);
	} else {
	    did = db.replace_document(urlterm, doc);
	}
	mark_as_seen(did);
	if (verbose) {
	    if (did <= old_lastdocid) {
		cout << "updated" << endl;
	    } else {
		cout << "added" << endl;
	    }
	}
    } else {
	// If this were a duplicate, we'd have skipped it above.
	db.add_document(doc);
	if (verbose)
	    cout << "added" << endl;
    }
}

void
index_mimetype(const string & file, const string & urlterm, const string & url,
	       const string & ext,
	       const string &mimetype, DirectoryIterator &d,
	       Xapian::Document & newdocument,
	       string record)
{
    string context(file, root.size(), string::npos);

    // FIXME: We could be cleverer here and check mtime too when use_ctime is
    // set - if the ctime has changed but the mtime is unchanged, we can just
    // update the existing Document and avoid having to re-extract text, etc.
    time_t last_altered = use_ctime ? d.get_ctime() : d.get_mtime();

    Xapian::docid did = 0;
    if (index_check_existing(urlterm, last_altered, did))
	return;

    if (!retry_failed) {
	// We only store and check the mtime (last modified) - a change to the
	// metadata won't generally cause a previous failure to now work
	// (FIXME: except permissions).
	time_t failed_last_mod;
	off_t failed_size;
	if (failed.contains(urlterm, failed_last_mod, failed_size)) {
	    if (d.get_mtime() <= failed_last_mod &&
		d.get_size() == failed_size) {
		if (verbose)
		    cout << "failed to extract text on earlier run" << endl;
		return;
	    }
	    // The file has changed, so remove the entry for it.  If it fails
	    // again on this attempt, we'll add a new one.
	    failed.del(urlterm);
	}
    }

    if (verbose) cout << flush;

    string author, title, sample, keywords, topic, dump;
    string md5;
    time_t created = time_t(-1);

    map<string, Filter>::const_iterator cmd_it = commands.find(mimetype);
    if (cmd_it == commands.end()) {
	size_t slash = mimetype.find('/');
	if (slash != string::npos) {
	    string wildtype(mimetype, 0, slash + 2);
	    wildtype[slash + 1] = '*';
	    cmd_it = commands.find(wildtype);
	    if (cmd_it == commands.end()) {
		cmd_it = commands.find("*/*");
	    }
	}
	if (cmd_it == commands.end()) {
	    cmd_it = commands.find("*");
	}
    }
    try {
	if (cmd_it != commands.end()) {
	    // Easy "run a command and read text or HTML from stdout or a
	    // temporary file" cases.
	    string cmd = cmd_it->second.cmd;
	    if (cmd.empty()) {
		skip(urlterm, context, "required filter not installed",
		     d.get_size(), d.get_mtime(), SKIP_VERBOSE_ONLY);
		return;
	    }
	    if (cmd == "false") {
		// Allow setting 'false' as a filter to mean that a MIME type
		// should be quietly ignored.
		string m = "ignoring MIME type '";
		m += cmd_it->first;
		m += "'";
		skip(urlterm, context, m, d.get_size(), d.get_mtime(),
		     SKIP_VERBOSE_ONLY);
		return;
	    }
	    bool use_shell = cmd_it->second.use_shell();
	    bool substituted = false;
	    string tmpout;
	    size_t pcent = 0;
	    while (true) {
		pcent = cmd.find('%', pcent);
		if (pcent >= cmd.size() - 1)
		    break;
		switch (cmd[pcent + 1]) {
		    case '%': // %% -> %.
			cmd.erase(++pcent, 1);
			break;
		    case 'f': { // %f -> escaped filename.
			substituted = true;
			string tail(cmd, pcent + 2);
			cmd.resize(pcent);
			append_filename_argument(cmd, file);
			// Remove the space append_filename_argument() adds before
			// the argument - the command string either includes one,
			// or won't expect one (e.g. --input=%f).
			cmd.erase(pcent, 1);
			pcent = cmd.size();
			cmd += tail;
			break;
		    }
		    case 't': { // %t -> temporary output file.
			if (tmpout.empty()) {
			    // Use a temporary file with a suitable extension
			    // in case the command cares, and for more helpful
			    // error messages from the command.
			    if (cmd_it->second.output_type == "text/html") {
				tmpout = get_tmpfile("tmp.html");
			    } else {
				tmpout = get_tmpfile("tmp.txt");
			    }
			}
			substituted = true;
			string tail(cmd, pcent + 2);
			cmd.resize(pcent);
			append_filename_argument(cmd, tmpout);
			// Remove the space append_filename_argument() adds before
			// the argument - the command string either includes one,
			// or won't expect one (e.g. --input=%f).
			cmd.erase(pcent, 1);
			pcent = cmd.size();
			cmd += tail;
			break;
		    }
		    default:
			// Leave anything else alone for now.
			pcent += 2;
			break;
		}
	    }
	    if (!substituted && cmd != "true") {
		// If no %f, append the filename to the command.
		append_filename_argument(cmd, file);
	    }
	    try {
		if (!tmpout.empty()) {
		    // Output in temporary file.
		    (void)stdout_to_string(cmd, use_shell);
		    if (!load_file(tmpout, dump)) {
			throw ReadError("Couldn't read output file");
		    }
		    unlink(tmpout.c_str());
		} else if (cmd == "true") {
		    // Ignore the file's contents, just index metadata from the
		    // filing system.
		} else {
		    // Output on stdout.
		    dump = stdout_to_string(cmd, use_shell);
		}
		const string & charset = cmd_it->second.output_charset;
		if (cmd_it->second.output_type == "text/html") {
		    MyHtmlParser p;
		    p.ignore_metarobots();
		    try {
			p.parse_html(dump, charset, false);
		    } catch (const string & newcharset) {
			p.reset();
			p.ignore_metarobots();
			p.parse_html(dump, newcharset, true);
		    } catch (ReadError) {
			skip_cmd_failed(urlterm, context, cmd,
					d.get_size(), d.get_mtime());
			return;
		    }
		    dump = p.dump;
		    title = p.title;
		    keywords = p.keywords;
		    topic = p.topic;
		    sample = p.sample;
		    author = p.author;
		    created = p.created;
		} else if (!charset.empty()) {
		    convert_to_utf8(dump, charset);
		}
	    } catch (ReadError) {
		skip_cmd_failed(urlterm, context, cmd,
				d.get_size(), d.get_mtime());
		return;
	    }
	} else if (mimetype == "text/html") {
	    const string & text = d.file_to_string();
	    MyHtmlParser p;
	    if (ignore_exclusions) p.ignore_metarobots();
	    try {
		// Default HTML character set is latin 1, though not specifying
		// one is deprecated these days.
		p.parse_html(text, "iso-8859-1", false);
	    } catch (const string & newcharset) {
		p.reset();
		if (ignore_exclusions) p.ignore_metarobots();
		p.parse_html(text, newcharset, true);
	    }
	    if (!p.indexing_allowed) {
		skip_meta_tag(urlterm, context,
			      d.get_size(), d.get_mtime());
		return;
	    }
	    dump = p.dump;
	    title = p.title;
	    keywords = p.keywords;
	    topic = p.topic;
	    sample = p.sample;
	    author = p.author;
	    created = p.created;
	    md5_string(text, md5);
	} else if (mimetype == "text/plain") {
	    // Currently we assume that text files are UTF-8 unless they have a
	    // byte-order mark.
	    dump = d.file_to_string();
	    md5_string(dump, md5);

	    // Look for Byte-Order Mark (BOM).
	    if (startswith(dump, "\xfe\xff") || startswith(dump, "\xff\xfe")) {
		// UTF-16 in big-endian/little-endian order - we just convert
		// it as "UTF-16" and let the conversion handle the BOM as that
		// way we avoid the copying overhead of erasing 2 bytes from
		// the start of dump.
		convert_to_utf8(dump, "UTF-16");
	    } else if (startswith(dump, "\xef\xbb\xbf")) {
		// UTF-8 with stupid Windows not-the-byte-order mark.
		dump.erase(0, 3);
	    } else {
		// FIXME: What charset is the file?  Look at contents?
	    }
	} else if (mimetype == "application/pdf") {
	    string cmd = "pdftotext -enc UTF-8";
	    append_filename_argument(cmd, file);
	    cmd += " -";
	    try {
		dump = stdout_to_string(cmd, false);
	    } catch (ReadError) {
		skip_cmd_failed(urlterm, context, cmd,
				d.get_size(), d.get_mtime());
		return;
	    }
	    get_pdf_metainfo(file, author, title, keywords, topic);
	} else if (mimetype == "application/postscript") {
	    // There simply doesn't seem to be a Unicode capable PostScript to
	    // text converter (e.g. pstotext always outputs ISO-8859-1).  The
	    // only solution seems to be to convert via PDF using ps2pdf and
	    // then pdftotext.  This gives plausible looking UTF-8 output for
	    // some Chinese PostScript files I found using Google.  It also has
	    // the benefit of allowing us to extract meta information from
	    // PostScript files.
	    string tmpfile = get_tmpfile("tmp.pdf");
	    if (tmpfile.empty()) {
		// FIXME: should this be fatal?  Or disable indexing postscript?
		string msg = "Couldn't create temporary directory (";
		msg += strerror(errno);
		msg += ")";
		skip(urlterm, context, msg,
		     d.get_size(), d.get_mtime());
		return;
	    }
	    string cmd = "ps2pdf";
	    append_filename_argument(cmd, file);
	    append_filename_argument(cmd, tmpfile);
	    try {
		(void)stdout_to_string(cmd, false);
		cmd = "pdftotext -enc UTF-8";
		append_filename_argument(cmd, tmpfile);
		cmd += " -";
		dump = stdout_to_string(cmd, false);
	    } catch (ReadError) {
		skip_cmd_failed(urlterm, context, cmd,
				d.get_size(), d.get_mtime());
		unlink(tmpfile.c_str());
		return;
	    } catch (...) {
		unlink(tmpfile.c_str());
		throw;
	    }
	    try {
		get_pdf_metainfo(tmpfile, author, title, keywords, topic);
	    } catch (...) {
		unlink(tmpfile.c_str());
		throw;
	    }
	    unlink(tmpfile.c_str());
	} else if (startswith(mimetype, "application/vnd.sun.xml.") ||
		   startswith(mimetype, "application/vnd.oasis.opendocument."))
	{
	    // Inspired by http://mjr.towers.org.uk/comp/sxw2text
	    string cmd = "unzip -p";
	    append_filename_argument(cmd, file);
	    cmd += " content.xml ; unzip -p";
	    append_filename_argument(cmd, file);
	    cmd += " styles.xml";
	    try {
		OpenDocParser parser;
		parser.parse(stdout_to_string(cmd, true));
		dump = parser.dump;
	    } catch (ReadError) {
		skip_cmd_failed(urlterm, context, cmd,
				d.get_size(), d.get_mtime());
		return;
	    }

	    cmd = "unzip -p";
	    append_filename_argument(cmd, file);
	    cmd += " meta.xml";
	    try {
		MetaXmlParser metaxmlparser;
		metaxmlparser.parse(stdout_to_string(cmd, false));
		title = metaxmlparser.title;
		keywords = metaxmlparser.keywords;
		// FIXME: topic = metaxmlparser.topic;
		sample = metaxmlparser.sample;
		author = metaxmlparser.author;
	    } catch (ReadError) {
		// It's probably best to index the document even if this fails.
	    }
	} else if (startswith(mimetype, "application/vnd.openxmlformats-officedocument.")) {
	    const char * args = NULL;
	    string tail(mimetype, 46);
	    if (startswith(tail, "wordprocessingml.")) {
		// unzip returns exit code 11 if a file to extract wasn't found
		// which we want to ignore, because there may be no headers or
		// no footers.
		args = " word/document.xml 'word/header*.xml' 'word/footer*.xml' 2>/dev/null";
	    } else if (startswith(tail, "spreadsheetml.")) {
		// Extract the shared string table first, so our parser can
		// grab those ready for parsing the sheets which will reference
		// the shared strings.
		string cmd = "unzip -p";
		append_filename_argument(cmd, file);
		cmd += " xl/styles.xml xl/workbook.xml xl/sharedStrings.xml ; unzip -p";
		append_filename_argument(cmd, file);
		cmd += " xl/worksheets/sheet\\*.xml";
		try {
		    XlsxParser parser;
		    parser.parse(stdout_to_string(cmd, true));
		    dump = parser.dump;
		} catch (ReadError) {
		    skip_cmd_failed(urlterm, context, cmd,
				    d.get_size(), d.get_mtime());
		    return;
		}
	    } else if (startswith(tail, "presentationml.")) {
		// unzip returns exit code 11 if a file to extract wasn't found
		// which we want to ignore, because there may be no notesSlides
		// or comments.
		args = " 'ppt/slides/slide*.xml' 'ppt/notesSlides/notesSlide*.xml' 'ppt/comments/comment*.xml' 2>/dev/null";
	    } else {
		// Don't know how to index this type.
		skip_unknown_mimetype(urlterm, context, mimetype,
				      d.get_size(), d.get_mtime());
		return;
	    }

	    if (args) {
		string cmd = "unzip -p";
		append_filename_argument(cmd, file);
		cmd += args;
		try {
		    MSXmlParser xmlparser;
		    // Treat exit status 11 from unzip as success - this is
		    // what we get if one of the listed filenames to extract
		    // doesn't match anything in the zip file.
		    xmlparser.parse_xml(stdout_to_string(cmd, false, 11));
		    dump = xmlparser.dump;
		} catch (ReadError) {
		    skip_cmd_failed(urlterm, context, cmd,
				    d.get_size(), d.get_mtime());
		    return;
		}
	    }

	    string cmd = "unzip -p";
	    append_filename_argument(cmd, file);
	    cmd += " docProps/core.xml";
	    try {
		MetaXmlParser metaxmlparser;
		metaxmlparser.parse(stdout_to_string(cmd, false));
		title = metaxmlparser.title;
		keywords = metaxmlparser.keywords;
		// FIXME: topic = metaxmlparser.topic;
		sample = metaxmlparser.sample;
		author = metaxmlparser.author;
	    } catch (ReadError) {
		// It's probably best to index the document even if this fails.
	    }
	} else if (mimetype == "application/x-abiword") {
	    // FIXME: Implement support for metadata.
	    XmlParser xmlparser;
	    const string & text = d.file_to_string();
	    xmlparser.parse_xml(text);
	    dump = xmlparser.dump;
	    md5_string(text, md5);
	} else if (mimetype == "application/x-abiword-compressed") {
	    // FIXME: Implement support for metadata.
	    XmlParser xmlparser;
	    xmlparser.parse_xml(d.gzfile_to_string());
	    dump = xmlparser.dump;
	} else if (mimetype == "application/vnd.ms-xpsdocument") {
	    string cmd = "unzip -p";
	    append_filename_argument(cmd, file);
	    cmd += " 'Documents/1/Pages/*.fpage'";
	    try {
		XpsXmlParser xpsparser;
		dump = stdout_to_string(cmd, false);
		// Look for Byte-Order Mark (BOM).
		if (startswith(dump, "\xfe\xff") || startswith(dump, "\xff\xfe")) {
		    // UTF-16 in big-endian/little-endian order - we just
		    // convert it as "UTF-16" and let the conversion handle the
		    // BOM as that way we avoid the copying overhead of erasing
		    // 2 bytes from the start of dump.
		    convert_to_utf8(dump, "UTF-16");
		}
		xpsparser.parse(dump);
		dump = xpsparser.dump;
	    } catch (ReadError) {
		skip_cmd_failed(urlterm, context, cmd,
				d.get_size(), d.get_mtime());
		return;
	    }
	} else if (mimetype == "text/csv") {
	    // Currently we assume that text files are UTF-8 unless they have a
	    // byte-order mark.
	    dump = d.file_to_string();
	    md5_string(dump, md5);

	    // Look for Byte-Order Mark (BOM).
	    if (startswith(dump, "\xfe\xff") || startswith(dump, "\xff\xfe")) {
		// UTF-16 in big-endian/little-endian order - we just convert
		// it as "UTF-16" and let the conversion handle the BOM as that
		// way we avoid the copying overhead of erasing 2 bytes from
		// the start of dump.
		convert_to_utf8(dump, "UTF-16");
	    } else if (startswith(dump, "\xef\xbb\xbf")) {
		// UTF-8 with stupid Windows not-the-byte-order mark.
		dump.erase(0, 3);
	    } else {
		// FIXME: What charset is the file?  Look at contents?
	    }

	    generate_sample_from_csv(dump, sample);
	} else if (mimetype == "image/svg+xml") {
	    SvgParser svgparser;
	    const string & text = d.file_to_string();
	    md5_string(text, md5);
	    svgparser.parse(text);
	    dump = svgparser.dump;
	    title = svgparser.title;
	    keywords = svgparser.keywords;
	    // FIXME: topic = svgparser.topic;
	    author = svgparser.author;
	} else if (mimetype == "application/x-debian-package") {
	    string cmd("dpkg-deb -f");
	    append_filename_argument(cmd, file);
	    cmd += " Description";
	    const string & desc = stdout_to_string(cmd, false);
	    // First line is short description, which we use as the title.
	    string::size_type idx = desc.find('\n');
	    title.assign(desc, 0, idx);
	    if (idx != string::npos) {
		dump.assign(desc, idx + 1, string::npos);
	    }
	} else if (mimetype == "application/x-redhat-package-manager") {
	    string cmd("rpm -q --qf '%{SUMMARY}\\n%{DESCRIPTION}' -p");
	    append_filename_argument(cmd, file);
	    const string & desc = stdout_to_string(cmd, false);
	    // First line is summary, which we use as the title.
	    string::size_type idx = desc.find('\n');
	    title.assign(desc, 0, idx);
	    if (idx != string::npos) {
		dump.assign(desc, idx + 1, string::npos);
	    }
	} else if (mimetype == "application/atom+xml") {
	    AtomParser atomparser;
	    const string & text = d.file_to_string();
	    md5_string(text, md5);
	    atomparser.parse(text);
	    dump = atomparser.dump;
	    title = atomparser.title;
	    keywords = atomparser.keywords;
	    // FIXME: topic = atomparser.topic;
	    author = atomparser.author;
	} else {
	    // Don't know how to index this type.
	    skip_unknown_mimetype(urlterm, context, mimetype,
				  d.get_size(), d.get_mtime());
	    return;
	}

	// Compute the MD5 of the file if we haven't already.
	if (md5.empty() && md5_file(file, md5, d.try_noatime()) == 0) {
	    if (errno == ENOENT || errno == ENOTDIR) {
		skip(urlterm, context, "File removed during indexing",
		     d.get_size(), d.get_mtime(),
		     SKIP_VERBOSE_ONLY | SKIP_SHOW_FILENAME);
	    } else {
		skip(urlterm, context, "failed to read file to calculate MD5 checksum",
		     d.get_size(), d.get_mtime());
	    }
	    return;
	}

	// Remove any trailing formfeeds, so we don't consider them when
	// considering if we extracted any text (e.g. pdftotext outputs a
	// formfeed between each page, even for blank pages).
	//
	// If dump contain only formfeeds, then trim_end will be string::npos
	// and ++trim_end will be 0, which is the correct new size.
	string::size_type trim_end = dump.find_last_not_of('\f');
	if (++trim_end != dump.size())
	    dump.resize(trim_end);

	if (dump.empty()) {
	    switch (empty_body) {
		case EMPTY_BODY_INDEX:
		    break;
		case EMPTY_BODY_WARN:
		    cout << "no text extracted from document body, "
			    "but indexing metadata anyway" << endl;
		    break;
		case EMPTY_BODY_SKIP:
		    skip(urlterm, context, "no text extracted from document body",
			 d.get_size(), d.get_mtime());
		    return;
	    }
	}

	// Produce a sample
	if (sample.empty()) {
	    sample = generate_sample(dump, sample_size, "...", " ...");
	} else {
	    sample = generate_sample(sample, sample_size, "...", " ...");
	}

	// Put the data in the document
	if (record.empty()) {
	    record = "url=";
	} else {
	    record += "\nurl=";
	}
	record += url;
	record += "\nsample=";
	record += sample;
	if (!title.empty()) {
	    record += "\ncaption=";
	    record += generate_sample(title, title_size, "...", " ...");
	}
	if (!author.empty()) {
	    record += "\nauthor=";
	    record += author;
	}
	record += "\ntype=";
	record += mimetype;
	time_t mtime = d.get_mtime();
	if (mtime != (time_t)-1) {
	    record += "\nmodtime=";
	    record += str(mtime);
	}
	if (created != (time_t)-1) {
	    record += "\ncreated=";
	    record += str(created);
	}
	off_t size = d.get_size();
	record += "\nsize=";
	record += str(size);
	newdocument.set_data(record);

	// Index the title, document text, keywords and topic.
	indexer.set_document(newdocument);
	if (!title.empty()) {
	    indexer.index_text(title, 5, "S");
	    indexer.increase_termpos(100);
	}
	if (!dump.empty()) {
	    indexer.index_text(dump);
	}
	if (!keywords.empty()) {
	    indexer.increase_termpos(100);
	    indexer.index_text(keywords);
	}
	if (!topic.empty()) {
	    indexer.increase_termpos(100);
	    indexer.index_text(topic, 1, "B");
	}
	// Index the leafname of the file.
	{
	    indexer.increase_termpos(100);
	    string leaf = d.leafname();
	    string::size_type dot = leaf.find_last_of('.');
	    if (dot != string::npos && leaf.size() - dot - 1 <= max_ext_len)
		leaf.resize(dot);
	    indexer.index_text(leaf, 1, "F");
	}

	if (!author.empty()) {
	    indexer.increase_termpos(100);
	    indexer.index_text(author, 1, "A");
	}

	// mimeType:
	newdocument.add_boolean_term("T" + mimetype);

	newdocument.add_boolean_term(site_term);

	if (!host_term.empty())
	    newdocument.add_boolean_term(host_term);

	struct tm *tm = localtime(&mtime);
	string date_term = "D" + date_to_string(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
	newdocument.add_boolean_term(date_term); // Date (YYYYMMDD)
	date_term.resize(7);
	date_term[0] = 'M';
	newdocument.add_boolean_term(date_term); // Month (YYYYMM)
	date_term.resize(5);
	date_term[0] = 'Y';
	newdocument.add_boolean_term(date_term); // Year (YYYY)

	newdocument.add_boolean_term(urlterm); // Url

	// Add mtime as a value to allow "sort by date".
	newdocument.add_value(VALUE_LASTMOD,
			      int_to_binary_string((uint32_t)mtime));
	if (use_ctime) {
	    // Add ctime as a value to track modifications.
	    time_t ctime = d.get_ctime();
	    newdocument.add_value(VALUE_CTIME,
				  int_to_binary_string((uint32_t)ctime));
	}

	// Add MD5 as a value to allow duplicate documents to be collapsed
	// together.
	newdocument.add_value(VALUE_MD5, md5);

	// Add the file size as a value to allow "sort by size" and size ranges.
	newdocument.add_value(VALUE_SIZE,
			      Xapian::sortable_serialise(size));

	bool inc_tag_added = false;
	if (d.is_other_readable()) {
	    inc_tag_added = true;
	    newdocument.add_boolean_term("I*");
	} else if (d.is_group_readable()) {
	    const char * group = d.get_group();
	    if (group) {
		newdocument.add_boolean_term(string("I#") + group);
	    }
	}
	const char * owner = d.get_owner();
	if (owner) {
	    newdocument.add_boolean_term(string("O") + owner);
	    if (!inc_tag_added && d.is_owner_readable())
		newdocument.add_boolean_term(string("I@") + owner);
	}

	string ext_term("E");
	for (string::const_iterator i = ext.begin(); i != ext.end(); ++i) {
	    char ch = *i;
	    if (ch >= 'A' && ch <= 'Z')
		ch |= 32;
	    ext_term += ch;
	}
	newdocument.add_boolean_term(ext_term);

	index_add_document(urlterm, last_altered, did, newdocument);
    } catch (ReadError) {
	skip(urlterm, context, string("can't read file: ") + strerror(errno),
	     d.get_size(), d.get_mtime());
    } catch (NoSuchFilter) {
	string filter_entry;
	if (cmd_it != commands.end()) {
	    filter_entry = cmd_it->first;
	} else {
	    filter_entry = mimetype;
	}
	string m = "Filter for \"";
	m += filter_entry;
	m += "\" not installed";
	skip(urlterm, context, m, d.get_size(), d.get_mtime());
	commands[filter_entry] = Filter();
    } catch (FileNotFound) {
	skip(urlterm, context, "File removed during indexing",
	     d.get_size(), d.get_mtime(),
	     SKIP_VERBOSE_ONLY | SKIP_SHOW_FILENAME);
    } catch (const std::string & error) {
	skip(urlterm, context, error, d.get_size(), d.get_mtime());
    }
}

void
index_handle_deletion()
{
    if (updated.empty() || old_docs_not_seen == 0) return;

    if (verbose) {
	cout << "Deleting " << old_docs_not_seen << " old documents which weren't found" << endl;
    }
    Xapian::PostingIterator alldocs = db.postlist_begin(string());
    Xapian::docid did = *alldocs;
    while (did < updated.size()) {
	if (!updated[did]) {
	    alldocs.skip_to(did);
	    if (alldocs == db.postlist_end(string()))
		break;
	    if (*alldocs != did) {
		// Document #did didn't exist before we started.
		did = *alldocs;
		continue;
	    }
	    db.delete_document(did);
	    if (--old_docs_not_seen == 0)
		break;
	}
	++did;
    }
}

void
index_commit()
{
    db.commit();
}

void
index_done()
{
    // If we created a temporary directory then delete it.
    remove_tmpdir();
}
