/** @file
 * @brief index static documents into the omega db
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2005 James Aylett
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2012,2013,2014,2015,2017,2018 Olly Betts
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

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <map>

#include <sys/types.h>
#include "safeunistd.h"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "safefcntl.h"

#ifdef HAVE_FNMATCH
# include <fnmatch.h>
#endif

#include <xapian.h>

#include "commonhelp.h"
#include "diritor.h"
#include "hashterm.h"
#include "index_file.h"
#include "mime.h"
#include "parseint.h"
#include "realtime.h"
#include "str.h"
#include "stringutils.h"
#include "urlencode.h"

#include "gnu_getopt.h"

using namespace std;

#define PROG_NAME "omindex"
#define PROG_DESC "Index static website data via the filesystem"

#define TITLE_SIZE 128
#define SAMPLE_SIZE 512

static bool follow_symlinks = false;
static off_t max_size = 0;
static std::string pretty_max_size;
static bool verbose = false;
static double sleep_before_opendir = 0;
static bool date_terms = false;

static string root;
static string url_start_path;

#ifdef HAVE_FNMATCH
static vector<pair<const char*, const char*>> mime_patterns;
#endif

static inline bool
p_notalnum(unsigned int c)
{
    return !C_isalnum(c);
}

static void
index_file(const string &file, const string &url, DirectoryIterator & d,
	   map<string, string>& mime_map)
{
    string urlterm("U");
    urlterm += url;

    if (urlterm.length() > MAX_SAFE_TERM_LENGTH)
	urlterm = hash_long_term(urlterm, MAX_SAFE_TERM_LENGTH);

    const char* leafname = d.leafname();

    string mimetype;
#ifdef HAVE_FNMATCH
    for (auto&& i : mime_patterns) {
	if (fnmatch(i.first, leafname, 0) == 0) {
	    if (strcmp(i.second, "ignore") == 0)
		return;
	    if (strcmp(i.second, "skip") == 0) {
		string m = "Leafname '";
		m += leafname;
		m += "' matches pattern: ";
		m += i.first;
		skip(urlterm, file.substr(root.size()), m,
		     d.get_size(), d.get_mtime());
		return;
	    }
	    mimetype = i.second;
	    break;
	}
    }
#endif

    string ext;
    const char * dot_ptr = strrchr(leafname, '.');
    if (dot_ptr) {
	ext.assign(dot_ptr + 1);
	if (ext.size() > max_ext_len)
	    ext.resize(0);
    }

    if (mimetype.empty()) {
	mimetype = mimetype_from_ext(mime_map, ext);
	if (mimetype == "ignore") {
	    // Remove any existing failed entry for this file.
	    index_remove_failed_entry(urlterm);
	    return;
	} else if (mimetype == "skip") {
	    // Ignore mimetype, skipped mimetype should not be quietly ignored.
	    string m = "skipping extension '";
	    m += ext;
	    m += "'";
	    skip(urlterm, file.substr(root.size()), m,
		 d.get_size(), d.get_mtime());
	    return;
	}
    }

    // Check the file size.
    off_t size = d.get_size();
    if (size == 0) {
	skip(urlterm, file.substr(root.size()), "Zero-sized file",
	     size, d.get_mtime(), SKIP_VERBOSE_ONLY);
	return;
    }

    if (max_size > 0 && size > max_size) {
	skip(urlterm, file.substr(root.size()),
	     "Larger than size limit of " + pretty_max_size,
	     size, d.get_mtime(),
	     SKIP_VERBOSE_ONLY);
	return;
    }

    string path_term("P");
    path_term += url_start_path;
    path_term.append(file, root.size(), string::npos);

    index_mimetype(file, urlterm, url, ext, mimetype, d, path_term, string());
}

static void
index_directory(const string &path, const string &url_, size_t depth_limit,
		map<string, string>& mime_map)
{
    if (verbose)
	cout << "[Entering directory \"" << path.substr(root.size()) << "\"]"
	     << endl;

    DirectoryIterator d(follow_symlinks);
    try {
	// Crude workaround for MS-DFS share misbehaviour.
	if (sleep_before_opendir > 0.0)
	    RealTime::sleep(RealTime::now() + sleep_before_opendir);

	d.start(path);

	while (d.next()) {
	    string url = url_;
	    url_encode(url, d.leafname());
	    string file = path;
	    file += d.leafname();

	    try {
		switch (d.get_type()) {
		    case DirectoryIterator::DIRECTORY: {
			size_t new_limit = depth_limit;
			if (new_limit) {
			    if (--new_limit == 0) continue;
			}
			url += '/';
			file += '/';
			index_directory(file, url, new_limit, mime_map);
			break;
		    }
		    case DirectoryIterator::REGULAR_FILE:
			index_file(file, url, d, mime_map);
			break;
		    default:
			skip("U" + url, file.substr(root.size()), "Not a regular file",
			     d.get_size(), d.get_mtime(),
			     SKIP_VERBOSE_ONLY | SKIP_SHOW_FILENAME);
		}
	    } catch (const FileNotFound & e) {
		skip("U" + url, file.substr(root.size()), "File removed during indexing",
		     d.get_size(), d.get_mtime(),
		     /*SKIP_VERBOSE_ONLY |*/ SKIP_SHOW_FILENAME);
	    } catch (const std::string & error) {
		skip("U" + url, file.substr(root.size()), error,
		     d.get_size(), d.get_mtime(), SKIP_SHOW_FILENAME);
	    }
	}
    } catch (const FileNotFound&) {
	if (verbose)
	    cout << "Directory \"" << path.substr(root.size()) << "\" "
		    "deleted during indexing" << endl;
    } catch (const std::string & error) {
	cout << error << " - skipping directory "
		"\"" << path.substr(root.size()) << "\"" << endl;
    }
}

static off_t
parse_size(char* p)
{
    // Don't want negative numbers, infinity, NaN, or hex numbers.
    if (C_isdigit(p[0]) && (p[1] | 32) != 'x') {
	double arg = strtod(p, &p);
	switch (*p) {
	    case '\0':
		break;
	    case 'k': case 'K':
		arg *= 1024;
		++p;
		break;
	    case 'm': case 'M':
		arg *= (1024 * 1024);
		++p;
		break;
	    case 'g': case 'G':
		arg *= (1024 * 1024 * 1024);
		++p;
		break;
	}
	if (*p == '\0') {
	    return off_t(arg);
	}
    }
    return -1;
}

static bool
parse_filter_rule(const char* rule, map<string, string>& mime_map)
{
    const char* s = strchr(rule, ':');
    if (s == NULL || s[1] == '\0') {
	cerr << "Invalid filter mapping '" << rule << "'\n"
		"Should be of the form TYPE:COMMAND or TYPE1,TYPE2:COMMAND or "
		"TYPE,EXT:COMMAND\n"
		"e.g. 'application/octet-stream:strings -n8'"
	     << endl;
	return false;
    }

    const char* c = static_cast<const char*>(memchr(rule, ',', s - rule));
    string output_type, output_charset;
    if (c) {
	// Filter produces a specified content-type.
	++c;
	const char* c2 = static_cast<const char *>(memchr(c, ',', s - c));
	if (c2) {
	    output_type.assign(c, c2 - c);
	    ++c2;
	    output_charset.assign(c2, s - c2);
	} else {
	    output_type.assign(c, s - c);
	}
	--c;
	if (output_type.find('/') == string::npos) {
	    auto m = mime_map.find(output_type);
	    if (m != mime_map.end()) {
		output_type = m->second;
	    } else {
		const char* r = built_in_mime_map(output_type);
		if (r) output_type = r;
	    }
	}
	if (output_type != "text/html" &&
	    output_type != "text/plain" &&
	    output_type != "image/svg+xml") {
	    cerr << "Currently only output types 'image/svg+xml', "
		    "'text/html' and 'text/plain' are supported."
		 << endl;
	    return false;
	}
    } else {
	c = s;
    }

    const char* cmd = s + 1;
    unsigned flags = 0;
    if (cmd[0] == '|') {
	flags |= Filter::PIPE_IN;
	++cmd;
	// FIXME: Do we need a way to set PIPE_DEV_STDIN and SEEK_DEV_STDIN?
	//
	// PIPE_DEV_STDIN doesn't seem to offer much over |foo2txt /dev/stdin
	// for user-specified filters (internally it provides a way to
	// gracefully handle platforms without /dev/stdin).
	//
	// SEEK_DEV_STDIN isn't currently easily approximated though.
    }
    // Analyse the command string to decide if it needs a shell.
    if (command_needs_shell(cmd))
	flags |= Filter::USE_SHELL;
    index_command(string(rule, c - rule),
		  Filter(string(cmd), output_type, output_charset, flags));

    return true;
}

int
main(int argc, char **argv)
{
    // If overwrite is true, the database will be created anew even if it
    // already exists.
    bool overwrite = false;
    // If delete_removed_documents is true, delete any documents we don't see.
    bool delete_removed_documents = true;
    // Retry files which we failed to index on a previous run?
    bool retry_failed = false;
    bool use_ctime = false;
    bool spelling = false;
    bool skip_duplicates = false;
    bool ignore_exclusions = false;
    bool description_as_sample = false;
    string baseurl;
    size_t depth_limit = 0;
    size_t title_size = TITLE_SIZE;
    size_t sample_size = SAMPLE_SIZE;
    empty_body_type empty_body = EMPTY_BODY_WARN;
    string site_term, host_term;
    Xapian::Stem stemmer("english");

    enum {
	OPT_OPENDIR_SLEEP = 256,
	OPT_SAMPLE,
	OPT_DATE_TERMS,
	OPT_READ_FILTERS
    };
    constexpr auto NO_ARG = no_argument;
    constexpr auto REQ_ARG = required_argument;
    static const struct option longopts[] = {
	{ "help",		NO_ARG,		NULL, 'h' },
	{ "version",		NO_ARG,		NULL, 'V' },
	{ "overwrite",		NO_ARG,		NULL, 'o' },
	{ "duplicates",		REQ_ARG,	NULL, 'd' },
	{ "no-delete",		NO_ARG,		NULL, 'p' },
	{ "db",			REQ_ARG,	NULL, 'D' },
	{ "url",		REQ_ARG,	NULL, 'U' },
	{ "mime-type",		REQ_ARG,	NULL, 'M' },
	{ "mime-type-match",	REQ_ARG,	NULL, 'G' },
	{ "filter",		REQ_ARG,	NULL, 'F' },
	{ "read-filters",	REQ_ARG,	NULL, OPT_READ_FILTERS },
	{ "depth-limit",	REQ_ARG,	NULL, 'l' },
	{ "follow",		NO_ARG,		NULL, 'f' },
	{ "ignore-exclusions",	NO_ARG,		NULL, 'i' },
	{ "stemmer",		REQ_ARG,	NULL, 's' },
	{ "spelling",		NO_ARG,		NULL, 'S' },
	{ "verbose",		NO_ARG,		NULL, 'v' },
	{ "empty-docs",		REQ_ARG,	NULL, 'e' },
	{ "max-size",		REQ_ARG,	NULL, 'm' },
	{ "sample",		REQ_ARG,	NULL, OPT_SAMPLE },
	{ "sample-size",	REQ_ARG,	NULL, 'E' },
	{ "title-size",		REQ_ARG,	NULL, 'T' },
	{ "retry-failed",	NO_ARG,		NULL, 'R' },
	{ "opendir-sleep",	REQ_ARG,	NULL, OPT_OPENDIR_SLEEP },
	{ "track-ctime",	NO_ARG,		NULL, 'C' },
	{ "date-terms",		NO_ARG,		NULL, OPT_DATE_TERMS },
	{ 0, 0, NULL, 0 }
    };

    map<string, string> mime_map;

    index_add_default_filters();
    index_add_default_libraries();

    if (argc == 2 && strcmp(argv[1], "-v") == 0) {
	// -v was the short option for --version in 1.2.3 and earlier, but
	// now it is short for --verbose (for consistency with scriptindex)
	// so if "-v" is the only option, translate it to "--version" for
	// backwards compatibility.
	argv[1] = const_cast<char *>("--version");
    }

    string dbpath;
    int getopt_ret;
    while ((getopt_ret = gnu_getopt_long(argc, argv, "hvd:D:U:M:F:l:s:pfRSVe:im:E:T:",
					 longopts, NULL)) != -1) {
	switch (getopt_ret) {
	case 'h': {
	    cout << PROG_NAME " - " PROG_DESC "\n\n"
"Usage: " PROG_NAME " [OPTIONS] --db DATABASE [BASEDIR] DIRECTORY\n"
"\n"
"DIRECTORY is the directory to start indexing from.\n"
"\n"
"BASEDIR is the directory corresponding to URL (default: DIRECTORY).\n"
"\n"
"Options:\n"
"  -d, --duplicates=ARG      set duplicate handling: ARG can be 'ignore' or\n"
"                            'replace' (default: replace)\n"
"  -p, --no-delete           skip the deletion of documents corresponding to\n"
"                            deleted files\n"
"  -e, --empty-docs=ARG      how to handle documents we extract no text from:\n"
"                            ARG can be index, warn (issue a diagnostic and\n"
"                            index), or skip.  (default: warn)\n"
"  -D, --db=DATABASE         path to database to use\n"
"  -U, --url=URL             base url BASEDIR corresponds to (default: /)\n"
"  -M, --mime-type=EXT:TYPE  assume any file with extension EXT has MIME\n"
"                            Content-Type TYPE, instead of using libmagic\n"
"                            (empty TYPE removes any existing mapping for EXT;\n"
"                            other special TYPE values: 'ignore' and 'skip')\n"
"  -G, --mime-type-match=GLOB:TYPE\n"
"                            assume any file with leaf name matching shell\n"
"                            wildcard pattern GLOB has MIME Content-Type TYPE\n"
"                            (special TYPE values: 'ignore' and 'skip')\n"
"  -F, --filter=M[,[T][,C]]:CMD\n"
"                            process files with MIME Content-Type M using\n"
"                            command CMD, which produces output (on stdout or\n"
"                            in a temporary file) with format T (Content-Type\n"
"                            or file extension; currently txt (default), html\n"
"                            or svg) in character encoding C (default: UTF-8).\n"
"                            E.g. -Fapplication/octet-stream:'|strings -n8'\n"
"                            or -Ftext/x-foo,,utf-16:'foo2utf16 %f %t'\n"
"      --read-filters=FILE   bulk-load --filter arguments from FILE, which\n"
"                            should contain one such argument per line (e.g.\n"
"                            text/x-bar:bar2txt --utf8).  Lines starting with #\n"
"                            are treated as comments and ignored.\n"
"  -l, --depth-limit=LIMIT   set recursion limit (0 = unlimited)\n"
"  -f, --follow              follow symbolic links\n"
"  -i, --ignore-exclusions   ignore meta robots tags and similar exclusions\n"
"  -S, --spelling            index data for spelling correction\n"
"  -m, --max-size            maximum size of file to index (in bytes or with a\n"
"                            suffix of 'K'/'k', 'M'/'m', 'G'/'g')\n"
"                            (default: unlimited)\n"
"      --sample=SOURCE       what to use for the stored sample of text for\n"
"                            HTML documents - SOURCE can be 'body' or\n"
"                            'description' (default: 'body')\n"
"  -E, --sample-size=SIZE    maximum size for the document text sample\n"
"                            (supports the same formats as --max-size).\n"
"                            (default: " STRINGIZE(SAMPLE_SIZE) ")\n"
"  -T, --title-size=SIZE     maximum size for the document title\n"
"                            (supports the same formats as --max-size).\n"
"                            (default: " STRINGIZE(TITLE_SIZE) ")\n"
"  -R, --retry-failed        retry files which omindex failed to extract text\n"
"                            from on a previous run\n"
"      --opendir-sleep=SECS  sleep for SECS seconds before opening each\n"
"                            directory - sleeping for 2 seconds seems to\n"
"                            reliably work around problems with indexing files\n"
"                            on Microsoft DFS shares.\n"
"  -C, --track-ctime         track each file's ctime so we can detect changes\n"
"                            to ownership or permissions.\n"
"  -v, --verbose             show more information about what is happening\n"
"      --overwrite           create the database anew (the default is to update\n"
"                            if the database already exists)" << endl;
	    print_stemmer_help("      ");
	    print_help_and_version_help("      ");
	    return 0;
	}
	case 'V':
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
	case 'e':
	    if (strcmp(optarg, "index") == 0) {
		empty_body = EMPTY_BODY_INDEX;
	    } else if (strcmp(optarg, "warn") == 0) {
		empty_body = EMPTY_BODY_WARN;
	    } else if (strcmp(optarg, "skip") == 0) {
		empty_body = EMPTY_BODY_SKIP;
	    } else {
		cerr << "Invalid --empty-docs value '" << optarg << "'\n"
			"Valid values are index, warn, and skip." << endl;
		return 1;
	    }
	    break;
	case 'p': // Keep documents even if the files have been removed.
	    delete_removed_documents = false;
	    break;
	case 'l': { // Set recursion limit
	    int arg;
	    if (!parse_signed(optarg, arg)) {
		throw "Recursion limit must be an integer";
	    }
	    if (arg < 0) arg = 0;
	    depth_limit = size_t(arg);
	    break;
	}
	case 'f': // Turn on following of symlinks
	    follow_symlinks = true;
	    break;
	case 'M': {
	    const char * s = strrchr(optarg, ':');
	    if (s == NULL) {
		cerr << "Invalid MIME mapping '" << optarg << "'\n"
			"Should be of the form EXT:TYPE, e.g. txt:text/plain\n"
			"(or txt: to delete a default mapping)" << endl;
		return 1;
	    }

	    // -Mtxt: results in an empty string, which effectively removes the
	    // default mapping for .txt files.
	    mime_map[string(optarg, s - optarg)] = string(s + 1);
	    max_ext_len = max(max_ext_len, strlen(s + 1));
	    break;
	}
	case 'F':
	    if (!parse_filter_rule(optarg, mime_map))
		return 1;
	    break;
	case OPT_READ_FILTERS: {
	    ifstream stream(optarg);
	    if (!stream) {
		cerr << "Unable to open filter file '" << optarg << "' "
			"(" << strerror(errno) << ')' << endl;
		return 1;
	    }
	    string rule;
	    bool all_ok = true;
	    while (getline(stream, rule)) {
		if (startswith(rule, '#')) continue;
		if (!parse_filter_rule(rule.c_str(), mime_map))
		    all_ok = false;
	    }
	    if (!all_ok)
		return 1;
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
	case 'i':
	    ignore_exclusions = true;
	    break;
	case 'R': // --retry-failed
	    retry_failed = true;
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
	case 'v':
	    verbose = true;
	    break;
	case 'E': {
	    off_t arg = parse_size(optarg);
	    if (arg >= 0) {
		sample_size = size_t(arg);
		break;
	    }
	    cerr << PROG_NAME": bad sample size '" << optarg << "'" << endl;
	    return 1;
	}
	case 'T': {
	    off_t arg = parse_size(optarg);
	    if (arg >= 0) {
		title_size = size_t(arg);
		break;
	    }
	    cerr << PROG_NAME": bad title size '" << optarg << "'" << endl;
	    return 1;
	}
	case 'm': {
	    off_t size = parse_size(optarg);
	    if (size >= 0) {
		max_size = size;
		const char * suffix;
		// Set lsb to the lowest set bit in max_size.
		off_t lsb = max_size & -max_size;
		if (lsb >= off_t(1L << 30)) {
		    size >>= 30;
		    suffix = "GB";
		} else if (lsb >= off_t(1L << 20)) {
		    size >>= 20;
		    suffix = "MB";
		} else if (lsb >= off_t(1L << 10)) {
		    size >>= 10;
		    suffix = "KB";
		} else {
		    suffix = "B";
		}
		pretty_max_size = str(size);
		pretty_max_size += suffix;
		break;
	    }
	    cerr << PROG_NAME": bad max size '" << optarg << "'" << endl;
	    return 1;
	}
	case OPT_OPENDIR_SLEEP: {
	    // Don't want negative numbers, infinity, NaN, or hex numbers.
	    char * p = optarg;
	    if (C_isdigit(p[0]) && (p[1] | 32) != 'x') {
		sleep_before_opendir = strtod(p, &p);
		if (*p == '\0')
		    break;
	    }
	    cerr << PROG_NAME": bad --opendir-sleep argument: "
		 "'" << optarg << "'" << endl;
	    return 1;
	}
	case OPT_SAMPLE:
	    if (strcmp(optarg, "description") == 0) {
		description_as_sample = true;
	    } else if (strcmp(optarg, "body") == 0) {
		description_as_sample = false;
	    } else {
		cerr << "Invalid --sample value '" << optarg << "'\n"
			"Valid values are body and description." << endl;
		return 1;
	    }
	    break;
	case 'C':
	    use_ctime = true;
	    break;
	case OPT_DATE_TERMS:
	    date_terms = true;
	    break;
	case 'G': {
	    char * s = strrchr(optarg, ':');
	    if (s == NULL) {
		cerr << "Invalid MIME mapping '" << optarg << "'\n"
			"Should be of the form GLOB:TYPE, e.g. *~:ignore"
		     << endl;
		return 1;
	    }
#ifndef HAVE_FNMATCH
	    cerr << "--mime-type-match isn't supported in this build because "
		    "the fnmatch() function wasn't found at configure time."
		 << endl;
	    return 1;
#else
	    if (s == optarg) {
		cerr << "--mime-type-match with an empty pattern can never "
			"match." << endl;
		return 1;
	    }
	    if (memchr(optarg, '/', s - optarg)) {
		cerr << "--mime-type-match only matches against the leaf "
			"filename so a pattern containing '/' can never match."
		     << endl;
		return 1;
	    }
	    const char* type = s + 1;
	    if (*type == '\0') {
		cerr << "--mime-type-match doesn't support an empty MIME type"
		     << endl;
		return 1;
	    }
	    *s = '\0';
	    mime_patterns.emplace_back(optarg, type);
	    break;
#endif
	}
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
	cerr << PROG_NAME": --url not specified, assuming '/'." << endl;
    }
    // baseurl must end in a '/'.
    if (!endswith(baseurl, '/')) {
	baseurl += '/';
    }

    // Site term (omits the trailing slash):
    site_term = "J";
    site_term.append(baseurl, 0, baseurl.size() - 1);
    if (site_term.size() > MAX_SAFE_TERM_LENGTH)
	site_term = hash_long_term(site_term, MAX_SAFE_TERM_LENGTH);

    // Host term, if the URL contains a hostname (omits any port number):
    string::size_type j;
    j = find_if(baseurl.begin(), baseurl.end(), p_notalnum) - baseurl.begin();
    if (j > 0 && baseurl.substr(j, 3) == "://" && j + 3 < baseurl.size()) {
	j += 3;
	// We must find a '/' - we ensured baseurl ended with a '/' above.
	string::size_type k = baseurl.find('/', j);
	url_start_path.assign(baseurl, k, string::npos);
	string::const_iterator l;
	l = find(baseurl.begin() + j, baseurl.begin() + k, ':');
	string::size_type host_len = l - baseurl.begin() - j;
	host_term = "H";
	host_term.append(baseurl, j, host_len);
	// DNS hostname limit is 253.
	if (host_term.size() > MAX_SAFE_TERM_LENGTH)
	    host_term = hash_long_term(host_term, MAX_SAFE_TERM_LENGTH);
    } else {
	url_start_path = baseurl;
    }

    if (optind >= argc || optind + 2 < argc) {
	cerr << PROG_NAME": you must specify a directory to index.\n"
"Do this either as a single directory (corresponding to the base URL)\n"
"or two directories - the first corresponding to the base URL and the second\n"
"a subdirectory of that to index." << endl;
	return 1;
    }

    root = argv[optind];
    if (root.empty()) {
	cerr << PROG_NAME": start directory can not be empty." << endl;
	return 1;
    }
    if (!endswith(root, '/')) {
	root += '/';
    }
    if (optind + 2 == argc) {
	string start_url = argv[optind + 1];
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
	root += start_url;
	url_encode_path(baseurl, start_url);
    }

    int exitcode = 1;
    try {
	index_init(dbpath, stemmer, root, site_term, host_term, empty_body,
		   (skip_duplicates ? DUP_SKIP : DUP_CHECK_LAZILY),
		   sample_size, title_size, max_ext_len,
		   overwrite, retry_failed, delete_removed_documents, verbose,
		   use_ctime, spelling, ignore_exclusions,
		   description_as_sample, date_terms);
	index_directory(root, baseurl, depth_limit, mime_map);
	index_handle_deletion();
	index_commit();
	exitcode = 0;
    } catch (const CommitAndExit &e) {
	cout << "Exception: " << e.what() << endl;
	cout << "Committing pending changes..." << endl;
	index_commit();
    } catch (const Xapian::Error &e) {
	cout << "Exception: " << e.get_description() << endl;
    } catch (const exception &e) {
	cout << "Exception: " << e.what() << endl;
    } catch (const string &s) {
	cout << "Exception: " << s << endl;
    } catch (const char *s) {
	cout << "Exception: " << s << endl;
    } catch (...) {
	cout << "Caught unknown exception" << endl;
    }

    index_done();

    return exitcode;
}
