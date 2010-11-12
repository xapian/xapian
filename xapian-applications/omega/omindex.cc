/* omindex.cc: index static documents into the omega db
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2005 James Aylett
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010 Olly Betts
 * Copyright 2009 Frank J Bruzzaniti
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

#include <algorithm>
#include <fstream>
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

#include "commonhelp.h"
#include "diritor.h"
#include "hashterm.h"
#include "md5wrap.h"
#include "metaxmlparse.h"
#include "myhtmlparse.h"
#include "pkglibbindir.h"
#include "runfilter.h"
#include "sample.h"
#include "str.h"
#include "stringutils.h"
#include "svgparse.h"
#include "utf8convert.h"
#include "utils.h"
#include "values.h"
#include "xmlparse.h"
#include "xpsxmlparse.h"

#include "gnu_getopt.h"

#ifndef HAVE_MKDTEMP
extern char * mkdtemp(char *);
#endif

using namespace std;

#define TITLE_SIZE 128
#define SAMPLE_SIZE 512

#define PROG_NAME "omindex"
#define PROG_DESC "Index static website data via the filesystem"

static bool skip_duplicates = false;
static bool follow_symlinks = false;
static bool spelling = false;
static bool verbose = false;

static string root;
static string baseurl;
static string site_term, host_term;
static Xapian::WritableDatabase db;
static Xapian::Stem stemmer("english");
static Xapian::TermGenerator indexer;

static Xapian::doccount old_docs_not_seen;
static Xapian::docid old_lastdocid;
static vector<bool> updated;

static string tmpdir;

static time_t last_mod_max;

// Commands which take a filename as the last argument, and output UTF-8
// text are common, so we handle these with a std::map.
static map<string, string> commands;

inline static bool
p_notalnum(unsigned int c)
{
    return !isalnum(static_cast<unsigned char>(c));
}

static string
shell_protect(const string & file)
{
    string safefile = file;
#ifdef __WIN32__
    bool need_to_quote = false;
    for (string::iterator i = safefile.begin(); i != safefile.end(); ++i) {
	unsigned char ch = *i;
	if (!isalnum(ch) && ch < 128) {
	    if (ch == '/') {
		// Convert Unix path separators to backslashes.  C library
		// functions understand "/" in paths, but external commands
		// generally don't, and also may interpret a leading '/' as
		// introducing a command line option.
		*i = '\\';
	    } else if (ch == ' ') {
		need_to_quote = true;
	    } else if (ch < 32 || strchr("<>\"|*?", ch)) {
		// Check for invalid characters in the filename.
		string m("Invalid character '");
		m += ch;
		m += "' in filename \"";
		m += file;
		m += '"';
		throw m;
	    }
	}
    }
    if (safefile[0] == '-') {
	// If the filename starts with a '-', protect it from being treated as
	// an option by prepending ".\".
	safefile.insert(0, ".\\");
    }
    if (need_to_quote) {
	safefile.insert(0, "\"");
	safefile += '"';
    }
#else
    string::size_type p = 0;
    if (!safefile.empty() && safefile[0] == '-') {
	// If the filename starts with a '-', protect it from being treated as
	// an option by prepending "./".
	safefile.insert(0, "./");
	p = 2;
    }
    while (p < safefile.size()) {
	// Don't escape some safe characters which are common in filenames.
	unsigned char ch = safefile[p];
	if (!isalnum(ch) && strchr("/._-", ch) == NULL) {
	    safefile.insert(p, "\\");
	    ++p;
	}
	++p;
    }
#endif
    return safefile;
}

static bool ensure_tmpdir() {
    if (!tmpdir.empty()) return true;

    const char * p = getenv("TMPDIR");
    if (!p) p = "/tmp";
    char * dir_template = new char[strlen(p) + 15 + 1];
    strcpy(dir_template, p);
    strcat(dir_template, "/omindex-XXXXXX");
    p = mkdtemp(dir_template);
    if (p) {
	tmpdir.assign(dir_template);
	tmpdir += '/';
    }
    delete [] dir_template;
    return (p != NULL);
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
get_pdf_metainfo(const string & safefile, string &author, string &title,
		 string &keywords)
{
    try {
	string pdfinfo = stdout_to_string("pdfinfo -enc UTF-8 " + safefile);

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
    // output is SAMPLE_SIZE - 1 bytes long.
    sample.reserve(SAMPLE_SIZE + 3);
    size_t last_word_end = 0;
    bool in_space = true;
    bool in_quotes = false;
    Xapian::Utf8Iterator i(csv_data);
    for ( ; i != Xapian::Utf8Iterator(); ++i) {
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

	if (sample.size() >= SAMPLE_SIZE) {
	    // Need to truncate sample.
	    if (last_word_end <= SAMPLE_SIZE / 2) {
		// Monster word!  We'll have to just split it.
		sample.replace(SAMPLE_SIZE - 3, string::npos, "...", 3);
	    } else {
		sample.replace(last_word_end, string::npos, " ...", 4);
	    }
	    break;
	}
    }
}

static void
index_file(const string &url, const string &mimetype, DirectoryIterator & d)
{
    string file = root + url;
    string author, title, sample, keywords, dump;

    if (verbose)
	cout << "Indexing \"" << url << "\" as " << mimetype << " ... ";

    string urlterm("U");
    urlterm += baseurl;
    urlterm += url;

    if (urlterm.length() > MAX_SAFE_TERM_LENGTH)
	urlterm = hash_long_term(urlterm, MAX_SAFE_TERM_LENGTH);

    time_t last_mod = d.get_mtime();

    Xapian::docid did = 0; 
    if (skip_duplicates) {
	Xapian::PostingIterator p = db.postlist_begin(urlterm);
	if (p != db.postlist_end(urlterm)) {
	    if (verbose)
		cout << "already indexed, not updating" << endl;
	    did = *p;
	    if (usual(did < updated.size() && !updated[did])) {
		updated[did] = true;
		--old_docs_not_seen;
	    }
	    return;
	}
    } else {
	// If last_mod > last_mod_max, we know for sure that the file is new
	// or updated.
	if (last_mod <= last_mod_max) {
	    Xapian::PostingIterator p = db.postlist_begin(urlterm);
	    if (p != db.postlist_end(urlterm)) {
		did = *p;
		Xapian::Document doc = db.get_document(did);
		string value = doc.get_value(VALUE_LASTMOD);
		time_t old_last_mod = binary_string_to_int(value);
		if (last_mod <= old_last_mod) {
		    if (verbose)
			cout << "already indexed" << endl;
		    // The docid should be in updated - the only valid
		    // exception is if the URL was long and hashed to the
		    // same URL as an existing document indexed in the same
		    // batch.
		    if (usual(did < updated.size() && !updated[did])) {
			updated[did] = true;
			--old_docs_not_seen;
		    }
		    return;
		}
	    }
	}
    }

    if (verbose) cout << flush;

    string md5;
    map<string, string>::const_iterator cmd_it = commands.find(mimetype);
    if (cmd_it != commands.end()) {
	// Easy "run a command and read UTF-8 text from stdout" cases.
	string cmd = cmd_it->second;
	cmd += shell_protect(file);
	try {
	    dump = stdout_to_string(cmd);
	} catch (ReadError) {
	    cout << "\"" << cmd << "\" failed - skipping" << endl;
	    return;
	}
    } else if (mimetype == "text/html") {
	string text;
	try {
	    text = d.file_to_string();
	} catch (ReadError) {
	    cout << "can't read \"" << file << "\" - skipping" << endl;
	    return;
	}
	MyHtmlParser p;
	try {
	    // Default HTML character set is latin 1, though not specifying one
	    // is deprecated these days.
	    p.parse_html(text, "iso-8859-1", false);
	} catch (const string & newcharset) {
	    try {
		p.reset();
		p.parse_html(text, newcharset, true);
	    } catch (bool) {
	    }
	} catch (bool) {
	    // MyHtmlParser throws a bool to abandon parsing at </body> or when
	    // indexing is disallowed
	}
	if (!p.indexing_allowed) {
	    cout << "indexing disallowed by meta tag - skipping" << endl;
	    return;
	}
	dump = p.dump;
	title = p.title;
	keywords = p.keywords;
	sample = p.sample;
	author = p.author;
	md5_string(text, md5);
    } else if (mimetype == "text/plain") {
	try {
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
	} catch (ReadError) {
	    cout << "can't read \"" << file << "\" - skipping" << endl;
	    return;
	}
    } else if (mimetype == "application/pdf") {
	string safefile = shell_protect(file);
	string cmd = "pdftotext -enc UTF-8 " + safefile + " -";
	try {
	    dump = stdout_to_string(cmd);
	} catch (ReadError) {
	    cout << "\"" << cmd << "\" failed - skipping" << endl;
	    return;
	}
	get_pdf_metainfo(safefile, author, title, keywords);
    } else if (mimetype == "application/postscript") {
	// There simply doesn't seem to be a Unicode capable PostScript to
	// text converter (e.g. pstotext always outputs ISO-8859-1).  The only
	// solution seems to be to convert via PDF using ps2pdf and then
	// pdftotext.  This gives plausible looking UTF-8 output for some
	// Chinese PostScript files I found using Google.  It also has the
	// benefit of allowing us to extract meta information from PostScript
	// files.
	if (!ensure_tmpdir()) {
	    // FIXME: should this be fatal?  Or disable indexing postscript?
	    cout << "Couldn't create temporary directory (" << strerror(errno) << ") - skipping" << endl;
	    return;
	}
	string tmpfile = tmpdir + "/tmp.pdf";
	string safetmp = shell_protect(tmpfile);
	string cmd = "ps2pdf " + shell_protect(file) + " " + safetmp;
	try {
	    (void)stdout_to_string(cmd);
	    cmd = "pdftotext -enc UTF-8 " + safetmp + " -";
	    dump = stdout_to_string(cmd);
	} catch (ReadError) {
	    cout << "\"" << cmd << "\" failed - skipping" << endl;
	    unlink(tmpfile.c_str());
	    return;
	} catch (...) {
	    unlink(tmpfile.c_str());
	    throw;
	}
	try {
	    get_pdf_metainfo(safetmp, author, title, keywords);
	} catch (...) {
	    unlink(tmpfile.c_str());
	    throw;
	}
	unlink(tmpfile.c_str());
    } else if (startswith(mimetype, "application/vnd.sun.xml.") ||
	       startswith(mimetype, "application/vnd.oasis.opendocument."))
    {
	// Inspired by http://mjr.towers.org.uk/comp/sxw2text
	string safefile = shell_protect(file);
	string cmd = "unzip -p " + safefile + " content.xml styles.xml";
	try {
	    XmlParser xmlparser;
	    xmlparser.parse_html(stdout_to_string(cmd));
	    dump = xmlparser.dump;
	} catch (ReadError) {
	    cout << "\"" << cmd << "\" failed - skipping" << endl;
	    return;
	}

	cmd = "unzip -p " + safefile + " meta.xml";
	try {
	    MetaXmlParser metaxmlparser;
	    metaxmlparser.parse_html(stdout_to_string(cmd));
	    title = metaxmlparser.title;
	    keywords = metaxmlparser.keywords;
	    sample = metaxmlparser.sample;
	    author = metaxmlparser.author;
	} catch (ReadError) {
	    // It's probably best to index the document even if this fails.
	}
    } else if (mimetype == "application/vnd.ms-excel") {
	string cmd = "xls2csv -q1 -dutf-8 " + shell_protect(file);
	try {
	    dump = stdout_to_string(cmd);
	} catch (ReadError) {
	    cout << "\"" << cmd << "\" failed - skipping" << endl;
	    return;
	}
	generate_sample_from_csv(dump, sample);
    } else if (startswith(mimetype, "application/vnd.openxmlformats-officedocument.")) {
	const char * args = NULL;
	string tail(mimetype, 46);
	if (startswith(tail, "wordprocessingml.")) {
	    // unzip returns exit code 11 if a file to extract wasn't found
	    // which we want to ignore, because there may be no headers or
	    // no footers.
	    args = " word/document.xml word/header\\*.xml word/footer\\*.xml 2>/dev/null||test $? = 11";
	} else if (startswith(tail, "spreadsheetml.")) {
	    args = " xl/sharedStrings.xml";
	} else if (startswith(tail, "presentationml.")) {
	    // unzip returns exit code 11 if a file to extract wasn't found
	    // which we want to ignore, because there may be no notesSlides
	    // or comments.
	    args = " ppt/slides/slide\\*.xml ppt/notesSlides/notesSlide\\*.xml ppt/comments/comment\\*.xml 2>/dev/null||test $? = 11";
	} else {
	    // Don't know how to index this type.
	    cout << "unknown Office 2007 MIME subtype - skipping" << endl;
	    return;
	}
	string safefile = shell_protect(file);
	string cmd = "unzip -p " + safefile + args;
	try {
	    XmlParser xmlparser;
	    xmlparser.parse_html(stdout_to_string(cmd));
	    dump = xmlparser.dump;
	} catch (ReadError) {
	    cout << "\"" << cmd << "\" failed - skipping" << endl;
	    return;
	}

	cmd = "unzip -p " + safefile + " docProps/core.xml";
	try {
	    MetaXmlParser metaxmlparser;
	    metaxmlparser.parse_html(stdout_to_string(cmd));
	    title = metaxmlparser.title;
	    keywords = metaxmlparser.keywords;
	    sample = metaxmlparser.sample;
	    author = metaxmlparser.author;
	} catch (ReadError) {
	    // It's probably best to index the document even if this fails.
	}
    } else if (mimetype == "application/x-abiword") {
	// FIXME: Implement support for metadata.
	try {
	    XmlParser xmlparser;
	    string text = d.file_to_string();
	    xmlparser.parse_html(text);
	    dump = xmlparser.dump;
	    md5_string(text, md5);
	} catch (ReadError) {
	    cout << "can't read \"" << file << "\" - skipping" << endl;
	    return;
	}
    } else if (mimetype == "application/x-abiword-compressed") {
	// FIXME: Implement support for metadata.
	string cmd = "gzip -dc " + shell_protect(file);
	try {
	    XmlParser xmlparser;
	    xmlparser.parse_html(stdout_to_string(cmd));
	    dump = xmlparser.dump;
	} catch (ReadError) {
	    cout << "\"" << cmd << "\" failed - skipping" << endl;
	    return;
	}
    } else if (mimetype == "text/rtf") {
	// The --text option unhelpfully converts all non-ASCII characters to
	// "?" so we use --html instead, which produces HTML entities.
	string cmd = "unrtf --nopict --html 2>/dev/null " + shell_protect(file);
	MyHtmlParser p;
	try {
	    // No point going looking for charset overrides as unrtf doesn't
	    // produce them.
	    p.parse_html(stdout_to_string(cmd), "iso-8859-1", true);
	} catch (ReadError) {
	    cout << "\"" << cmd << "\" failed - skipping" << endl;
	    return;
	} catch (bool) {
	    // MyHtmlParser throws a bool to abandon parsing at </body> or when
	    // indexing is disallowed
	}
	if (!p.indexing_allowed) {
	    cout << "indexing disallowed by meta tag - skipping" << endl;
	    return;
	}
	dump = p.dump;
	title = p.title;
	keywords = p.keywords;
	sample = p.sample;
    } else if (mimetype == "text/x-perl") {
	// pod2text's output character set doesn't seem to be documented, but
	// from inspecting the source it looks like it's probably iso-8859-1.
	string cmd = "pod2text " + shell_protect(file);
	try {
	    dump = stdout_to_string(cmd);
	    convert_to_utf8(dump, "ISO-8859-1");
	} catch (ReadError) {
	    cout << "\"" << cmd << "\" failed - skipping" << endl;
	    return;
	}
    } else if (mimetype == "application/x-dvi") {
	// FIXME: -e0 means "UTF-8", but that results in "fi", "ff", "ffi", etc
	// appearing as single ligatures.  For European languages, it's
	// actually better to use -e2 (ISO-8859-1) and then convert, so let's
	// do that for now until we handle Unicode "compatibility
	// decompositions".
	string cmd = "catdvi -e2 -s " + shell_protect(file);
	try {
	    dump = stdout_to_string(cmd);
	    convert_to_utf8(dump, "ISO-8859-1");
	} catch (ReadError) {
	    cout << "\"" << cmd << "\" failed - skipping" << endl;
	    return;
	}
    } else if (mimetype == "application/vnd.ms-xpsdocument") {
	string safefile = shell_protect(file);
	string cmd = "unzip -p " + safefile + " Documents/1/Pages/\\*.fpage";
	try {
	    XpsXmlParser xpsparser;
	    dump = stdout_to_string(cmd);
	    // Look for Byte-Order Mark (BOM).
	    if (startswith(dump, "\xfe\xff") || startswith(dump, "\xff\xfe")) {
		// UTF-16 in big-endian/little-endian order - we just convert
		// it as "UTF-16" and let the conversion handle the BOM as that
		// way we avoid the copying overhead of erasing 2 bytes from
		// the start of dump.
		convert_to_utf8(dump, "UTF-16");
	    }
	    xpsparser.parse_html(dump);
	    dump = xpsparser.dump;
	} catch (ReadError) {
	    cout << "\"" << cmd << "\" failed - skipping" << endl;
	    return;
	}
    } else if (mimetype == "text/csv") {
	try {
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
	} catch (ReadError) {
	    cout << "can't read \"" << file << "\" - skipping" << endl;
	    return;
	}
    } else if (mimetype == "application/vnd.ms-outlook") {
	string cmd = get_pkglibdindir() + "/outlookmsg2html " + shell_protect(file);
	MyHtmlParser p;
	try {
	    dump = stdout_to_string(cmd);
	    // FIXME: what should the default charset be?
	    p.parse_html(dump, "iso-8859-1", false);
	} catch (const string & newcharset) {
	    try {
		p.reset();
		p.parse_html(dump, newcharset, true);
	    } catch (bool) {
	    }
	} catch (ReadError) {
	    cout << "\"" << cmd << "\" failed - skipping" << endl;
	    return;
	} catch (bool) {
	    // MyHtmlParser throws a bool to abandon parsing at </body> or when
	    // indexing is disallowed
	}
	if (!p.indexing_allowed) {
	    cout << "indexing disallowed by meta tag - skipping" << endl;
	    return;
	}
	dump = p.dump;
	title = p.title;
	keywords = p.keywords;
	sample = p.sample;
	author = p.author;
    } else if (mimetype == "image/svg+xml") {
	SvgParser svgparser;
	svgparser.parse_html(d.file_to_string());
	dump = svgparser.dump;
	title = svgparser.title;
	keywords = svgparser.keywords;
	author = svgparser.author;
    } else if (mimetype == "application/x-debian-package") {
	string cmd("dpkg-deb -f ");
	cmd += shell_protect(file);
	cmd += " Description";
	const string & desc = stdout_to_string(cmd);
	// First line is short description, which we use as the title.
	string::size_type idx = desc.find('\n');
	title.assign(desc, 0, idx);
	if (idx != string::npos) {
	    dump.assign(desc, idx + 1, string::npos);
	}
    } else if (mimetype == "application/x-redhat-package-manager") {
	string cmd("rpm -q --qf '%{SUMMARY}\\n%{DESCRIPTION}' -p ");
	cmd += shell_protect(file);
	const string & desc = stdout_to_string(cmd);
	// First line is summary, which we use as the title.
	string::size_type idx = desc.find('\n');
	title.assign(desc, 0, idx);
	if (idx != string::npos) {
	    dump.assign(desc, idx + 1, string::npos);
	}
    } else {
	// Don't know how to index this type.
	cout << "unknown MIME type - skipping" << endl;
	return;
    }

    // Compute the MD5 of the file if we haven't already.
    if (md5.empty() && md5_file(file, md5, d.try_noatime()) == 0) {
	cout << "failed to read file to calculate MD5 checksum - skipping" << endl;
	return;
    }

    // Produce a sample
    if (sample.empty()) {
	sample = generate_sample(dump, SAMPLE_SIZE);
    } else {
	sample = generate_sample(sample, SAMPLE_SIZE);
    }

    // Put the data in the document
    Xapian::Document newdocument;
    string record = "url=" + baseurl + url + "\nsample=" + sample;
    if (!title.empty()) {
	record += "\ncaption=" + generate_sample(title, TITLE_SIZE);
    }
    if (!author.empty()) {
	record += "\nauthor=";
	record += author;
    }
    record += "\ntype=" + mimetype;
    if (last_mod != (time_t)-1) {
	record += "\nmodtime=";
	record += str(last_mod);
    }
    record += "\nsize=";
    record += str(d.get_size());
    newdocument.set_data(record);

    // Index the title, document text, and keywords.
    indexer.set_document(newdocument);
    if (!title.empty()) {
	indexer.index_text(title, 5);
	indexer.increase_termpos(100);
    }
    if (!dump.empty()) {
	indexer.index_text(dump);
    }
    if (!keywords.empty()) {
	indexer.increase_termpos(100);
	indexer.index_text(keywords);
    }
    // FIXME: index author too

    // mimeType:
    newdocument.add_boolean_term("T" + mimetype);

    newdocument.add_boolean_term(site_term);

    if (!host_term.empty())
	newdocument.add_boolean_term(host_term);

    struct tm *tm = localtime(&last_mod);
    string date_term = "D" + date_to_string(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    newdocument.add_boolean_term(date_term); // Date (YYYYMMDD)
    date_term.resize(7);
    date_term[0] = 'M';
    newdocument.add_boolean_term(date_term); // Month (YYYYMM)
    date_term.resize(5);
    date_term[0] = 'Y';
    newdocument.add_boolean_term(date_term); // Year (YYYY)

    newdocument.add_boolean_term(urlterm); // Url

    // Add last_mod as a value to allow "sort by date".
    newdocument.add_value(VALUE_LASTMOD,
			  int_to_binary_string((uint32_t)last_mod));

    // Add MD5 as a value to allow duplicate documents to be collapsed together.
    newdocument.add_value(VALUE_MD5, md5);

    // Add the file size as a value to allow "sort by size" and size ranges.
    newdocument.add_value(VALUE_SIZE, Xapian::sortable_serialise(d.get_size()));

    bool inc_tag_added = false;
    if (d.is_other_readable()) {
	inc_tag_added = true;
	newdocument.add_boolean_term("I*");
    } else if (d.is_group_readable()) {
	const char * group = d.get_group();
	if (group) {
	    newdocument.add_boolean_term(string("I#") + group);
	    inc_tag_added = true;
	}
    }
    const char * owner = d.get_owner();
    if (owner) {
	newdocument.add_boolean_term(string("O") + owner);
	if (!inc_tag_added && d.is_owner_readable())
	    newdocument.add_boolean_term(string("I@") + owner);
    }

    if (!skip_duplicates) {
	// If this document has already been indexed, update the existing
	// entry.
	if (did) {
	    // We already found out the document id above.
	    db.replace_document(did, newdocument);
	} else if (last_mod <= last_mod_max) {
	    // We checked for the UID term and didn't find it.
	    did = db.add_document(newdocument);
	} else {
	    did = db.replace_document(urlterm, newdocument);
	}
	if (did < updated.size()) {
	    if (usual(!updated[did])) {
		updated[did] = true;
		--old_docs_not_seen;
	    }
	}
	if (verbose) {
	    if (did < old_lastdocid) {
		cout << "updated" << endl;
	    } else {
		cout << "added" << endl;
	    }
	}
    } else {
	// If this were a duplicate, we'd have skipped it above.
	db.add_document(newdocument);
	if (verbose)
	    cout << "added" << endl;
    }
}

static void
index_directory(size_t depth_limit, const string &dir,
		map<string, string>& mime_map)
{
    string path = root + dir;

    if (verbose)
	cout << "[Entering directory \"" << dir << "\"]" << endl;

    DirectoryIterator d(follow_symlinks);
    try {
	d.start(path);
    } catch (const std::string & error) {
	cout << error << " - skipping directory \"" << dir << "\"" << endl;
	return;
    }

    while (d.next()) try {
	string url = dir;
	url += d.leafname();
	string file = root + url;
	switch (d.get_type()) {
	    case DirectoryIterator::DIRECTORY: {
		size_t new_limit = depth_limit;
		if (new_limit) {
		    if (--new_limit == 0) continue;
		}
		url += '/';
		index_directory(new_limit, url, mime_map);
		continue;
	    }
	    case DirectoryIterator::REGULAR_FILE: {
		string ext;
		string::size_type dot = url.find_last_of('.');
		if (dot != string::npos) ext = url.substr(dot + 1);

		map<string,string>::iterator mt = mime_map.find(ext);
		if (mt == mime_map.end()) {
		    // If the extension isn't found, see if the lower-cased
		    // version (if different) is found.
		    bool changed = false;
		    string::iterator i;
		    for (i = ext.begin(); i != ext.end(); ++i) {
			if (*i >= 'A' && *i <= 'Z') {
			    *i = tolower(*i);
			    changed = true;
			}
		    }
		    if (changed) mt = mime_map.find(ext);
		}
		if (mt != mime_map.end()) {
		    if (mt->second.empty()) {
			if (verbose) {
			    cout << "Skipping file, required filter not "
				    "installed: \"" << file << "\""
				 << endl;
			}
			continue;
		    }
		    if (mt->second == "ignore")
			continue;
		}

		string mimetype;
		if (mt == mime_map.end()) {
		    mimetype = d.get_magic_mimetype();
		    if (mimetype.empty()) {
			cout << "Unknown extension and unrecognised format: "
				"\"" << file << "\" - skipping" << endl;
			continue;
		    }
		    cout << "magic told us content-type [" << mimetype << "]" << endl;
//		    cout << "Unknown extension: \"" << file << "\" - "
//			    "skipping" << endl;
//		    continue;
		} else {
		    mimetype = mt->second;
		}

		// Only check the file size if we recognise the extension to
		// avoid a call to stat()/lstat() for files we can't handle
		// when readdir() tells us the file type.
		off_t size = d.get_size();
		if (size == 0) {
		    if (verbose) {
			cout << "Skipping empty file: \"" << file << "\""
			     << endl;
		    }
		    continue;
		}

		// It's in our MIME map so we know how to index it.
		try {
		    index_file(url, mimetype, d);
		} catch (NoSuchFilter) {
		    // FIXME: we ought to ignore by mime-type not extension.
		    cout << "Filter for \"" << mimetype << "\" not installed "
			    "- ignoring extension \"" << ext << "\"" << endl;
		    mt->second = string();
		}
		continue;
	    }
	    default:
		if (verbose) {
		    cout << "Not a regular file \"" << file << "\" - "
			    "skipping" << endl;
		}
	}
    } catch (const std::string & error) {
	cout << error << " - skipping" << endl;
	continue;
    }
}

int
main(int argc, char **argv)
{
    // If overwrite is true, the database will be created anew even if it
    // already exists.
    bool overwrite = false;
    // If delete_removed_documents is true, delete any documents we don't see.
    bool delete_removed_documents = true;
    size_t depth_limit = 0;

    static const struct option longopts[] = {
	{ "help",	no_argument,		NULL, 'h' },
	{ "version",	no_argument,		NULL, 'v' },
	{ "overwrite",	no_argument,		NULL, 'o' },
	{ "duplicates",	required_argument,	NULL, 'd' },
	{ "preserve-removed",	no_argument,	NULL, 'p' },
	{ "preserve-nonduplicates",	no_argument,	NULL, 'p' },
	{ "db",		required_argument,	NULL, 'D' },
	{ "url",	required_argument,	NULL, 'U' },
	{ "mime-type",	required_argument,	NULL, 'M' },
	{ "filter",	required_argument,	NULL, 'F' },
	{ "depth-limit",required_argument,	NULL, 'l' },
	{ "follow",	no_argument,		NULL, 'f' },
	{ "stemmer",	required_argument,	NULL, 's' },
	{ "spelling",	no_argument,		NULL, 'S' },
	{ "verbose",	no_argument,		NULL, 'V' },
	{ 0, 0, NULL, 0 }
    };

    map<string, string> mime_map;
    // Plain text:
    mime_map["txt"] = "text/plain";
    mime_map["text"] = "text/plain";

    // HTML:
    mime_map["html"] = "text/html";
    mime_map["htm"] = "text/html";
    mime_map["shtml"] = "text/html";
    mime_map["php"] = "text/html"; // Our HTML parser knows to ignore PHP code.

    // Comma-Separated Values:
    mime_map["csv"] = "text/csv";

    // PDF:
    mime_map["pdf"] = "application/pdf";

    // PostScript:
    mime_map["ps"] = "application/postscript";
    mime_map["eps"] = "application/postscript";
    mime_map["ai"] = "application/postscript";

    // OpenDocument:
    // FIXME: need to find sample documents to test all of these.
    mime_map["odt"] = "application/vnd.oasis.opendocument.text";
    mime_map["ods"] = "application/vnd.oasis.opendocument.spreadsheet";
    mime_map["odp"] = "application/vnd.oasis.opendocument.presentation";
    mime_map["odg"] = "application/vnd.oasis.opendocument.graphics";
    mime_map["odc"] = "application/vnd.oasis.opendocument.chart";
    mime_map["odf"] = "application/vnd.oasis.opendocument.formula";
    mime_map["odb"] = "application/vnd.oasis.opendocument.database";
    mime_map["odi"] = "application/vnd.oasis.opendocument.image";
    mime_map["odm"] = "application/vnd.oasis.opendocument.text-master";
    mime_map["ott"] = "application/vnd.oasis.opendocument.text-template";
    mime_map["ots"] = "application/vnd.oasis.opendocument.spreadsheet-template";
    mime_map["otp"] = "application/vnd.oasis.opendocument.presentation-template";
    mime_map["otg"] = "application/vnd.oasis.opendocument.graphics-template";
    mime_map["otc"] = "application/vnd.oasis.opendocument.chart-template";
    mime_map["otf"] = "application/vnd.oasis.opendocument.formula-template";
    mime_map["oti"] = "application/vnd.oasis.opendocument.image-template";
    mime_map["oth"] = "application/vnd.oasis.opendocument.text-web";

    // OpenOffice/StarOffice documents:
    mime_map["sxc"] = "application/vnd.sun.xml.calc";
    mime_map["stc"] = "application/vnd.sun.xml.calc.template";
    mime_map["sxd"] = "application/vnd.sun.xml.draw";
    mime_map["std"] = "application/vnd.sun.xml.draw.template";
    mime_map["sxi"] = "application/vnd.sun.xml.impress";
    mime_map["sti"] = "application/vnd.sun.xml.impress.template";
    mime_map["sxm"] = "application/vnd.sun.xml.math";
    mime_map["sxw"] = "application/vnd.sun.xml.writer";
    mime_map["sxg"] = "application/vnd.sun.xml.writer.global";
    mime_map["stw"] = "application/vnd.sun.xml.writer.template";

    // MS Office 2007 formats:
    mime_map["docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document"; // Word 2007
    mime_map["dotx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.template"; // Word 2007 template
    mime_map["xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"; // Excel 2007
    mime_map["xltx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.template"; // Excel 2007 template
    mime_map["pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation"; // PowerPoint 2007 presentation
    mime_map["ppsx"] = "application/vnd.openxmlformats-officedocument.presentationml.slideshow"; // PowerPoint 2007 slideshow
    mime_map["potx"] = "application/vnd.openxmlformats-officedocument.presentationml.template"; // PowerPoint 2007 template
    mime_map["xps"] = "application/vnd.ms-xpsdocument";

    // Macro-enabled variants - these appear to be the same formats as the
    // above.  Currently we just treat them as the same mimetypes to avoid
    // having to check for twice as many possible content-types.
    // MS say: application/vnd.ms-word.document.macroEnabled.12
    mime_map["docm"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    // MS say: application/vnd.ms-word.template.macroEnabled.12
    mime_map["dotm"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.template";
    // MS say: application/vnd.ms-excel.sheet.macroEnabled.12
    mime_map["xlsm"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    // MS say: application/vnd.ms-excel.template.macroEnabled.12
    mime_map["xltm"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.template";
    // MS say: application/vnd.ms-powerpoint.presentation.macroEnabled.12
    mime_map["pptm"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
    // MS say: application/vnd.ms-powerpoint.slideshow.macroEnabled.12
    mime_map["ppsm"] = "application/vnd.openxmlformats-officedocument.presentationml.slideshow";
    // MS say: application/vnd.ms-powerpoint.presentation.macroEnabled.12
    mime_map["potm"] = "application/vnd.openxmlformats-officedocument.presentationml.template";

    // Some other word processor formats:
    mime_map["doc"] = "application/msword";
    mime_map["dot"] = "application/msword"; // Word template
    mime_map["wpd"] = "application/vnd.wordperfect";
    mime_map["wps"] = "application/vnd.ms-works";
    mime_map["wpt"] = "application/vnd.ms-works"; // Works template
    mime_map["abw"] = "application/x-abiword"; // AbiWord
    mime_map["zabw"] = "application/x-abiword-compressed"; // AbiWord compressed
    mime_map["rtf"] = "text/rtf";

    // Other MS formats:
    mime_map["xls"] = "application/vnd.ms-excel";
    mime_map["xlb"] = "application/vnd.ms-excel";
    mime_map["xlt"] = "application/vnd.ms-excel"; // Excel template
    mime_map["ppt"] = "application/vnd.ms-powerpoint";
    mime_map["pps"] = "application/vnd.ms-powerpoint"; // Powerpoint slideshow
    mime_map["msg"] = "application/vnd.ms-outlook"; // Outlook .msg email

    // Perl:
    mime_map["pl"] = "text/x-perl";
    mime_map["pm"] = "text/x-perl";
    mime_map["pod"] = "text/x-perl";

    // TeX DVI:
    mime_map["dvi"] = "application/x-dvi";

    // DjVu:
    mime_map["djv"] = "image/vnd.djvu";
    mime_map["djvu"] = "image/vnd.djvu";

    // SVG:
    mime_map["svg"] = "image/svg+xml";

    // Debian packages:
    mime_map["deb"] = "application/x-debian-package";
    mime_map["udeb"] = "application/x-debian-package";

    // RPM packages:
    mime_map["rpm"] = "application/x-redhat-package-manager";

    // Extensions to quietly ignore:
    mime_map["a"] = "ignore";
    mime_map["dll"] = "ignore";
    mime_map["dylib"] = "ignore";
    mime_map["exe"] = "ignore";
    mime_map["lib"] = "ignore";
    mime_map["o"] = "ignore";
    mime_map["obj"] = "ignore";
    mime_map["so"] = "ignore";

    commands["application/msword"] = "antiword -mUTF-8.txt ";
    commands["application/vnd.ms-powerpoint"] = "catppt -dutf-8 ";
    // Looking at the source of wpd2html and wpd2text I think both output
    // UTF-8, but it's hard to be sure without sample Unicode .wpd files
    // as they don't seem to be at all well documented.
    commands["application/vnd.wordperfect"] = "wpd2text ";
    // wps2text produces UTF-8 output from the sample files I've tested.
    commands["application/vnd.ms-works"] = "wps2text ";
    // Output is UTF-8 according to "man djvutxt".  Generally this seems to
    // be true, though some examples from djvu.org generate isolated byte
    // 0x95 in a context which suggests it might be intended to be a bullet
    // (as it is in CP1250).
    commands["image/vnd.djvu"] = "djvutxt ";

    string dbpath;
    int getopt_ret;
    while ((getopt_ret = gnu_getopt_long(argc, argv, "hvd:D:U:M:F:l:s:pfSV",
					 longopts, NULL)) != -1) {
	switch (getopt_ret) {
	case 'h': {
	    cout << PROG_NAME" - "PROG_DESC"\n\n"
"Usage: "PROG_NAME" [OPTIONS] --db DATABASE [BASEDIR] DIRECTORY\n\n"
"Options:\n"
"  -d, --duplicates         set duplicate handling ('ignore' or 'replace')\n"
"  -p, --preserve-removed   keep documents in the database which seem to have\n"
"                           been removed from disk\n"
"      --preserve-nonduplicates  deprecated alias for -p\n"
"  -D, --db                 path to database to use\n"
"  -U, --url                base url DIRECTORY represents (default: /)\n"
"  -M, --mime-type=EXT:TYPE map file extension EXT to MIME Content-Type TYPE\n"
"                           (empty TYPE removes any MIME mapping for EXT)\n"
"  -F, --filter=TYPE:CMD    process files with MIME Content-Type TYPE using\n"
"                           command CMD, which should produce UTF-8 text on\n"
"                           stdout e.g. -Fapplication/octet-stream:'strings -n8'\n"
"  -l, --depth-limit=LIMIT  set recursion limit (0 = unlimited)\n"
"  -f, --follow             follow symbolic links\n"
"  -S, --spelling           index data for spelling correction\n"
"  -V, --verbose            show more information about what is happening\n"
"      --overwrite          create the database anew (the default is to update\n"
"                           if the database already exists)" << endl;
	    print_stemmer_help("     ");
	    print_help_and_version_help("     ");
	    return 0;
	}
	case 'v':
	    print_package_info(PROG_NAME);
	    return 0;
	case 'd': // how shall we handle duplicate documents?
	    switch (optarg[0]) {
	    case 'i':
		skip_duplicates = true;
		break;
	    case 'r':
		skip_duplicates = false;
		break;
	    }
	    break;
	case 'p': // Keep documents even if the files have been removed.
	    delete_removed_documents = false;
	    break;
	case 'l': { // Set recursion limit
	    int arg = atoi(optarg);
	    if (arg < 0) arg = 0;
	    depth_limit = size_t(arg);
	    break;
	}
	case 'f': // Turn on following of symlinks
	    follow_symlinks = true;
	    break;
	case 'M': {
	    const char * s = strchr(optarg, ':');
	    if (s != NULL) {
		if (s[1]) {
		    mime_map[string(optarg, s - optarg)] = string(s + 1);
		} else {
		    // -Mtxt: removes the default mapping for .txt files.
		    mime_map.erase(string(optarg, s - optarg));
		}
	    } else {
		cerr << "Invalid MIME mapping '" << optarg << "'\n"
			"Should be of the form ext:type, eg txt:text/plain\n"
			"(or txt: to delete a default mapping)" << endl;
		return 1;
	    }
	    break;
	}
	case 'F': {
	    const char * s = strchr(optarg, ':');
	    if (s != NULL || !s[1]) {
		string command(s + 1);
		command += ' ';
		commands[string(optarg, s - optarg)] = command;
	    } else {
		cerr << "Invalid filter mapping '" << optarg << "'\n"
			"Should be of the form TYPE:COMMAND, e.g. 'application/octet-stream:strings -n8'"
		     << endl;
		return 1;
	    }
	    break;
	}
	case 'D':
	    dbpath = optarg;
	    break;
	case 'U':
	    baseurl = optarg;
	    break;
	case 'o': // --overwrite
	    overwrite = true;
	    break;
	case 's':
	    try {
		stemmer = Xapian::Stem(optarg);
	    } catch (const Xapian::InvalidArgumentError &) {
		cerr << "Unknown stemming language '" << optarg << "'.\n"
			"Available language names are: "
		     << Xapian::Stem::get_available_languages() << endl;
		return 1;
	    }
	    break;
	case 'S':
	    spelling = true;
	    break;
	case 'V':
	    verbose = true;
	    break;
	case ':': // missing param
	    return 1;
	case '?': // unknown option: FIXME -> char
	    return 1;
	}
    }

    if (dbpath.empty()) {
	cerr << PROG_NAME": you must specify a database with --db." << endl;
	return 1;
    }
    if (baseurl.empty()) {
	cerr << PROG_NAME": --url not specified, assuming `/'." << endl;
    }
    // baseurl must end in a '/'.
    if (!endswith(baseurl, '/')) {
	baseurl += '/';
    }

    if (optind >= argc || optind + 2 < argc) {
	cerr << PROG_NAME": you must specify a directory to index.\n"
"Do this either as a single directory (corresponding to the base URL)\n"
"or two directories - the first corresponding to the base URL and the second\n"
"a subdirectory of that to index." << endl;
	return 1;
    }
    root = argv[optind];
    if (!endswith(root, '/')) {
	root += '/';
    }
    string start_url;
    if (optind + 2 == argc) {
	start_url = argv[optind + 1];
	if (startswith(start_url, '/')) {
	    // Make relative to root.
	    if (!startswith(start_url, root)) {
		cerr << PROG_NAME": '" << argv[optind + 1] << "' "
		    "is not a subdirectory of '" << argv[optind] << "'."
		     << endl;
		return 1;
	    }
	    start_url.erase(0, root.size());
	}
	if (!endswith(start_url, '/')) {
	    start_url += '/';
	}
    }

    string::size_type j;
    j = find_if(baseurl.begin(), baseurl.end(), p_notalnum) - baseurl.begin();
    if (j > 0 && baseurl.substr(j, 3) == "://") {
	j += 3;
	string::size_type k = baseurl.find('/', j);
	if (k == string::npos) {
	    // Path:
	    site_term = "P/";
	    // Host:
	    host_term = "H" + baseurl.substr(j);
	} else {
	    // Path:
	    string::size_type path_len = baseurl.size() - k;
	    // Subtract one to lose the trailing /, unless it's the initial / too.
	    if (path_len > 1) --path_len;
	    site_term = "P" + baseurl.substr(k, path_len);
	    // Host:
	    string::const_iterator l;
	    l = find(baseurl.begin() + j, baseurl.begin() + k, ':');
	    string::size_type host_len = l - baseurl.begin() - j;
	    host_term = "H" + baseurl.substr(j, host_len);
	}
    } else {
	// Path:
	string::size_type path_len = baseurl.size();
	// Subtract one to lose the trailing /, unless it's the initial / too.
	if (path_len > 1) --path_len;
	site_term = "P" + baseurl.substr(0, path_len);
    }

    int exitcode = 1;
    try {
	if (!overwrite) {
	    db = Xapian::WritableDatabase(dbpath, Xapian::DB_CREATE_OR_OPEN);
	    old_docs_not_seen = db.get_doccount();
	    old_lastdocid = db.get_lastdocid();
	    if (delete_removed_documents) {
		// + 1 so that old_lastdocid is a valid subscript.
		updated.resize(old_lastdocid + 1);
	    }
	    try {
		string ubound = db.get_value_upper_bound(VALUE_LASTMOD);
		if (!ubound.empty()) 
		    last_mod_max = binary_string_to_int(ubound);
	    } catch (const Xapian::UnimplementedError &) {
		numeric_limits<time_t> n;
		last_mod_max = n.max();
	    }
	} else {
	    db = Xapian::WritableDatabase(dbpath, Xapian::DB_CREATE_OR_OVERWRITE);
	}

	if (spelling) {
	    indexer.set_database(db);
	    indexer.set_flags(indexer.FLAG_SPELLING);
	}
	indexer.set_stemmer(stemmer);

	index_directory(depth_limit, start_url, mime_map);
	if (delete_removed_documents && old_docs_not_seen) {
	    if (verbose) {
		cout << "Deleting " << old_docs_not_seen << " old documents which weren't found" << endl;
	    }
	    Xapian::PostingIterator alldocs = db.postlist_begin(string());
	    Xapian::docid did = *alldocs;
	    do {
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
	    } while (++did < updated.size());
	}
	db.commit();
	exitcode = 0;
    } catch (const Xapian::Error &e) {
	cout << "Exception: " << e.get_description() << endl;
    } catch (const string &s) {
	cout << "Exception: " << s << endl;
    } catch (const char *s) {
	cout << "Exception: " << s << endl;
    } catch (...) {
	cout << "Caught unknown exception" << endl;
    }

    // If we created a temporary directory then delete it.
    if (!tmpdir.empty()) rmdir(tmpdir.c_str());

    return exitcode;
}
