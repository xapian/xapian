/* omindex.cc: index static documents into the omega db
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 James Aylett
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include <config.h>

#ifdef HAVE_POSIX_FADVISE
# ifdef __linux__
#  define _POSIX_C_SOURCE 200112L // for posix_fadvise from fcntl.h
# endif
#endif
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <vector>

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <xapian.h>

#include "myhtmlparse.h"
#include "indextext.h"

#include "gnu_getopt.h"

#ifndef O_STREAMING
# ifdef __linux__
// This is the value used by rml's O_STREAMING patch for 2.4.
#  define O_STREAMING	04000000
# else
// Define as 0 otherwise, so we don't need ifdefs in the code.
#  define O_STREAMING	0
# endif
#endif

using namespace std;

#define OMINDEX "omindex"

#define DUPE_ignore 0
#define DUPE_replace 1
#define DUPE_duplicate 2
static int dupes = DUPE_replace;
static int recurse = 1;
static bool follow_symlinks = false;
static string dbpath;
static string root;
static string indexroot;
static string baseurl;
static Xapian::WritableDatabase db;

vector<bool> updated;

static const unsigned int MAX_URL_LENGTH = 240;

inline static bool
p_notalnum(unsigned int c)
{
    return !isalnum(c);
}

/* Hash is computed as an unsigned long, and then converted to a
 * string by writing 6 bits of it to each output byte.  So length is
 * ceil(sizeof(unsigned long) * 8 / 6).
 */
#define HASH_LEN ((sizeof(unsigned long) * 8 + 5) / 6)

/* Make a hash of a string - this isn't a very good hashing algorithm, but
 * it's fast.  A collision would result in a document overwriting a different
 * document, which is not desirable, but also wouldn't be a total disaster.
 */
static string
hash_string(const string &s)
{
    unsigned long int h = 1;
    for (string::const_iterator i = s.begin(); i != s.end(); ++i) {
	h += (h << 5) + static_cast<unsigned long int>(*i);
    }
    string result = string(HASH_LEN, ' ');
    int i = 0;
    while (h != 0) {
	char ch = (char)((h & 63) + 33);
	result[i++] = ch;
	h = h >> 6;
    }
    return result;
}

/* Make a term for a url, ensuring that it's not longer than the maximum
 * length.  The term is "U"+baseurl+url, unless this would be too long, in
 * which case it is truncated to the maximum length - the length of the hash,
 * and has a hash of the truncated part appended.
 */
static string
make_url_term(const string &url)
{
    string result = "U" + baseurl + url;
    if (result.length() > MAX_URL_LENGTH) {
	result = result.substr(0, MAX_URL_LENGTH - HASH_LEN) +
		 hash_string(result.substr(MAX_URL_LENGTH - HASH_LEN));
	//printf("Using '%s' as the url term\n", result.c_str());
    }
    return result;
}

/* Truncate a string to a given maxlength, avoiding cutting off midword
 * if reasonably possible. */
string
truncate_to_word(string & input, string::size_type maxlen)
{
    string output;
    if (input.length() <= maxlen) {
	output = input;
    } else {
	output = input.substr(0, maxlen);

	string::size_type space = output.find_last_of(WHITESPACE);
	if (space != string::npos && space > maxlen / 2) {
	    string::size_type nonspace;
	    nonspace = output.find_last_not_of(WHITESPACE, space);
	    if (nonspace != string::npos) output.erase(nonspace);
	}

	if (output.length() == maxlen && !isspace(input[maxlen])) {
	    output += "...";
	} else {
	    output += " ...";
	}
    }

    // replace newlines with spaces
    size_t i = 0;    
    while ((i = output.find('\n', i)) != string::npos) output[i] = ' ';
    return output;
}

static string
shell_protect(const string & file)
{
    string safefile = file;
    string::size_type p = 0;
    if (!safefile.empty() && safefile[0] == '-') {
	// If the filename starts with a '-', protect it from being treated as
	// an option by prepending "./".
	safefile.insert(0, "./");
	p = 2;
    }
    while (p < safefile.size()) {
	// Exclude a few safe characters which are common in filenames
	if (!isalnum(safefile[p]) && strchr("/._-", safefile[p]) == NULL) {
	    safefile.insert(p, "\\");
	    ++p;
	}
	++p;
    }
    return safefile;
}

struct ReadError {};

static string
file_to_string(const string &file)
{
    string out;
    struct stat st;
    int fd = open(file.c_str(), O_RDONLY|O_STREAMING);
    if (fd == -1) throw ReadError();
    if (fstat(fd, &st) == -1 || !S_ISREG(st.st_mode)) {
	close(fd);
	throw ReadError();
    }

    if (st.st_size > 0) {
#ifdef HAVE_POSIX_FADVISE
	posix_fadvise(fd, 0, 0, POSIX_FADV_NOREUSE);
#endif
	out.reserve(st.st_size);
	char blk[4096];
	while (true) {
	    int r = read(fd, blk, sizeof(blk));
	    if (r < 0) {
		if (errno == EINTR) continue;
		close(fd);
		throw ReadError();
	    }
	    if (r == 0) break; // end of file
	    out.append(blk, r);
	}
    }
#ifdef HAVE_POSIX_FADVISE
    posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);
#endif
    close(fd);
    return out;
}

static string
stdout_to_string(const string &cmd)
{
    string out;
    FILE * fh = popen(cmd.c_str(), "r");
    if (fh == NULL) return out;
    while (!feof(fh)) {
	char buf[4096];
	size_t len = fread(buf, 1, 4096, fh);
	if (ferror(fh)) {
	    (void)fclose(fh);
	    throw ReadError();
	}
	out.append(buf, len);
    }
    if (fclose(fh) == -1) throw ReadError();
    return out;
}

static void
index_file(const string &url, const string &mimetype, time_t last_mod)
{
    string file = root + url;
    string title, sample, keywords, dump;
    string urlterm;

    cout << "Indexing \"" << url << "\" as " << mimetype << " ... ";

    urlterm = make_url_term(url);

    if (dupes == DUPE_ignore && db.term_exists(urlterm)) {
	cout << "duplicate. Ignored." << endl;
	return;
    }

    if (mimetype == "text/html") {
	string text;
	try {
	    text = file_to_string(file);
	} catch (ReadError) {
	    cout << "can't read \"" << file << "\" - skipping\n";
	    return;
	}
	MyHtmlParser p;
	try {
	    p.parse_html(text);
	} catch (bool) {
	    // MyHtmlParser throws a bool to abandon parsing at </body> or when
	    // indexing is disallowed
	}
	if (!p.indexing_allowed) {
	    cout << "indexing disallowed by meta tag - skipping\n";
	    return;
	}
	dump = p.dump;
	title = p.title;
	keywords = p.keywords;
	sample = p.sample;
    } else if (mimetype == "text/plain") {
	try {
	    dump = file_to_string(file);
	} catch (ReadError) {
	    cout << "can't read \"" << file << "\" - skipping\n";
	    return;
	}
    } else if (mimetype == "application/pdf") {
	string safefile = shell_protect(file);
	string cmd = "pdftotext " + safefile + " -";
	try {
	    dump = stdout_to_string(cmd);
	} catch (ReadError) {
	    cout << "\"" << cmd << "\" failed - skipping\n";
	    return;
	}

	// FIXME: run pdfinfo once and parse the output ourselves
	try {
	    title = stdout_to_string("pdfinfo " + safefile +
				     "|sed 's/^Title: *//p;d'");
	} catch (ReadError) {
	    title = "";
	}
	    
	try {
	    keywords = stdout_to_string("pdfinfo " + safefile +
					"|sed 's/^Keywords: *//p;d'");
	} catch (ReadError) {
	    keywords = "";
	}
    } else if (mimetype == "application/postscript") {
	string cmd = "pstotext " + shell_protect(file);
	try {
	    dump = stdout_to_string(cmd);
	} catch (ReadError) {
	    cout << "\"" << cmd << "\" failed - skipping\n";
	    return;
	}
    } else if (mimetype.substr(0, 24) == "application/vnd.sun.xml.") {
	// Inspired by http://mjr.towers.org.uk/comp/sxw2text
	string safefile = shell_protect(file);
#define OOO_XML_SED_DECODE_ENTITIES \
	"'s/&lt;/</g;s/&gt;/>/g;s/&apos;/'\\''/g;s/&quot;/\"/g;s/&amp;/\\&/g'"
	string cmd = "unzip -p " + safefile + " content.xml"
		     "|sed 's/<[^>]*>/ /g;'"OOO_XML_SED_DECODE_ENTITIES;
	try {
	    dump = stdout_to_string(cmd);
	} catch (ReadError) {
	    cout << "\"" << cmd << "\" failed - skipping\n";
	    return;
	}

	// FIXME: unzip meta.xml once and parse the output ourselves
	try {
	    cmd = "unzip -p " + safefile + " meta.xml"
		  "|sed 's/.*<dc:title>\\([^<]*\\).*/\\1/p;d'"
		  "|sed "OOO_XML_SED_DECODE_ENTITIES;
	    title = stdout_to_string(cmd);
	} catch (ReadError) {
	    title = "";
	}

	try {
	    // e.g.:
	    // <meta:keywords>
	    // <meta:keyword>information retrieval</meta:keyword>
	    // </meta:keywords>
	    cmd = "unzip -p " + safefile + " meta.xml"
		  "|sed 's/.*<meta:keywords>//;s!</meta:keywords>.*!!;"
			"s/<[^>]*>/ /g;'"OOO_XML_SED_DECODE_ENTITIES;
	    keywords = stdout_to_string(cmd);
	} catch (ReadError) {
	    keywords = "";
	}

	try {
	    // dc:subject is "Subject and Keywords":
	    // "Typically, Subject will be expressed as keywords, key phrases
	    // or classification codes that describe a topic of the resource."
	    // OpenOffice uses meta:keywords for keywords - dc:subject
	    // comes from a text field labelled "Subject.  Let's just treat
	    // it as more keywords.
	    cmd = "unzip -p " + safefile + " meta.xml"
		  "|sed 's/.*<dc:subject>\\([^<]*\\).*/\\1/p;d'"
		  "|sed "OOO_XML_SED_DECODE_ENTITIES;
	    string subject = stdout_to_string(cmd);
	    if (!subject.empty()) {
		keywords += ' ';
		keywords += subject;
	    }
	} catch (ReadError) {
	}

	try {
	    cmd = "unzip -p " + safefile + " meta.xml"
		  "|sed 's/.*<dc:description>\\([^<]*\\).*/\\1/p;d'"
		  "|sed "OOO_XML_SED_DECODE_ENTITIES;
	    sample = stdout_to_string(cmd);
	} catch (ReadError) {
	    sample = "";
	}
    } else if (mimetype == "application/msword") {
	string cmd = "antiword " + shell_protect(file);
	try {
	    dump = stdout_to_string(cmd);
	} catch (ReadError) {
	    cout << "\"" << cmd << "\" failed - skipping\n";
	    return;
	}
    } else if (mimetype == "application/vnd.wordperfect") {
	string cmd = "wpd2text " + shell_protect(file);
	try {
	    dump = stdout_to_string(cmd);
	} catch (ReadError) {
	    cout << "\"" << cmd << "\" failed - skipping\n";
	    return;
	}
    } else if (mimetype == "text/rtf") {
	string cmd = "unrtf --nopict --text 2>/dev/null " +
		     shell_protect(file) +
		     "|sed '/^### .*/d'";
	try {
	    dump = stdout_to_string(cmd);
	} catch (ReadError) {
	    cout << "\"" << cmd << "\" failed - skipping\n";
	    return;
	}
    } else {
	// Don't know how to index this
	cout << "unknown MIME type - skipping\n";
	return;
    }
    Xapian::Stem stemmer("english");

    // Produce a sample
    if (sample.empty()) {
	sample = truncate_to_word(dump, 300);
    } else {
	sample = truncate_to_word(sample, 300);
    }

    // Put the data in the document
    Xapian::Document newdocument;
    string record = "url=" + baseurl + url + "\nsample=" + sample;
    if (!title.empty()) {
	record += "\ncaption=" + truncate_to_word(title, 100);
    }
    record += "\ntype=" + mimetype;
    newdocument.set_data(record);

    // Add postings for terms to the document
    Xapian::termpos pos = 1;
    pos = index_text(title, newdocument, stemmer, pos);
    pos = index_text(dump, newdocument, stemmer, pos + 100);
    pos = index_text(keywords, newdocument, stemmer, pos + 100);

    newdocument.add_term("T" + mimetype); // mimeType
    string::size_type j;
    j = find_if(baseurl.begin(), baseurl.end(), p_notalnum) - baseurl.begin();
    if (j > 0 && baseurl.substr(j, 3) == "://") {
	j += 3;
	string::size_type k = baseurl.find('/', j);
	if (k == string::npos) {
	    newdocument.add_term("P/"); // Path
	    newdocument.add_term("H" + baseurl.substr(j));
	} else {
	    newdocument.add_term("P" + baseurl.substr(k)); // Path
	    string::const_iterator l;
	    l = find(baseurl.begin() + j, baseurl.begin() + k, ':');
	    string::size_type host_len = l - baseurl.begin() - j;
	    newdocument.add_term("H" + baseurl.substr(j, host_len)); // Host
	}
    } else {
	newdocument.add_term("P" + baseurl); // Path
    }

    struct tm *tm = localtime(&last_mod);
    char buf[9];
    sprintf(buf, "%04d%02d%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    newdocument.add_term("D" + string(buf)); // Date (YYYYMMDD)
    buf[7] = '\0';
    if (buf[6] == '3') buf[6] = '2';
    newdocument.add_term("W" + string(buf)); // "Weak" - 10ish day interval
    buf[6] = '\0';
    newdocument.add_term("M" + string(buf)); // Month (YYYYMM)
    buf[4] = '\0';
    newdocument.add_term("Y" + string(buf)); // Year (YYYY)
    newdocument.add_term(urlterm); // Url

    if (dupes == DUPE_replace) {
	// If this document has already been indexed, update the existing
	// entry.
	try {
	    Xapian::docid did = db.replace_document(urlterm, newdocument);
	    if (did < updated.size()) {
		updated[did] = true;
		cout << "updated." << endl;
	    } else {
		cout << "added." << endl;
	    }
	} catch (...) {
	    // FIXME: is this ever actually needed?
	    db.add_document(newdocument);
	    cout << "added (failed re-seek for duplicate)." << endl;
	}
    } else {
	db.add_document(newdocument);
	cout << "added." << endl;
    }
}

static void
index_directory(const string &dir, const map<string, string>& mime_map)
{
    DIR *d;
    struct dirent *ent;
    string path = root + indexroot + dir;

    cout << "[Entering directory " << dir << "]" << endl;

    d = opendir(path.c_str());
    if (d == NULL) {
	cout << "Can't open directory \"" << path << "\" - skipping\n";
	return;
    }
    while ((ent = readdir(d)) != NULL) {
	struct stat statbuf;
	// ".", "..", and other hidden files
	if (ent->d_name[0] == '.') continue;
	string url = dir;
	if (!url.empty() && url[url.size() - 1] != '/') url += '/';
	url += ent->d_name;
	string file = root + indexroot + url;
#ifdef HAVE_LSTAT
        if (follow_symlinks) {
#endif
            if (stat(file.c_str(), &statbuf) == -1) {
                cout << "Can't stat \"" << file << "\" - skipping\n";
                continue;
            }
#ifdef HAVE_LSTAT
        } else {
            if (lstat(file.c_str(), &statbuf) == -1) {
                cout << "Can't stat \"" << file << "\" - skipping\n";
                continue;
            }
        }
#endif
	if (S_ISDIR(statbuf.st_mode)) {
	    if (!recurse) continue;
	    try {
		index_directory(url, mime_map);
	    }
	    catch (...) {
		cout << "Caught unknown exception in index_directory, rethrowing" << endl;
		throw;
	    }
	    continue;
	}
	if (S_ISREG(statbuf.st_mode)) {
	    string ext;
	    string::size_type dot = url.find_last_of('.');
	    if (dot != string::npos) ext = url.substr(dot + 1);

	    map<string,string>::const_iterator mt;
	    if ((mt = mime_map.find(ext))!=mime_map.end()) {
	      // If it's in our MIME map, presumably we know how to index it
	      index_file(indexroot + url, mt->second, statbuf.st_mtime);
	    }
	    continue;
	}
	cout << "Not a regular file \"" << file << "\" - skipping\n";
    }
    closedir(d);
}

int
main(int argc, char **argv)
{
    // If overwrite is true, the database will be created anew even if it
    // already exists.
    bool overwrite = false;

    static const struct option longopts[] = {
	{ "help",	no_argument,		NULL, 'h' },
	{ "version",	no_argument,		NULL, 'v' },
	{ "overwrite",	no_argument,		NULL, 'o' },
	{ "duplicates",	required_argument,	NULL, 'd' },
	{ "db",		required_argument,	NULL, 'D' },
	{ "url",	required_argument,	NULL, 'U' },
	{ "mime-type",	required_argument,	NULL, 'M' },
	{ "no-recurse",	no_argument,		NULL, 'l' },
	{ "follow",	no_argument,		NULL, 'f' },
	{ 0, 0, NULL, 0 }
    };

    int getopt_ret;

    map<string, string> mime_map;
    mime_map["txt"] = "text/plain";
    mime_map["text"] = "text/plain";
    mime_map["html"] = "text/html";
    mime_map["htm"] = "text/html";
    mime_map["shtml"] = "text/html";
    mime_map["php"] = "text/html"; // Our HTML parser knows to ignore PHP
    mime_map["pdf"] = "application/pdf";
    mime_map["ps"] = "application/postscript";
    mime_map["eps"] = "application/postscript";
    mime_map["ai"] = "application/postscript";
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
    mime_map["doc"] = "application/msword";
    mime_map["wpd"] = "application/vnd.wordperfect";
    mime_map["rtf"] = "text/rtf";

    while ((getopt_ret = gnu_getopt_long(argc, argv, "hvd:D:U:M:l", longopts, NULL))!=EOF) {
	switch (getopt_ret) {
	case 'h':
	    cout << OMINDEX << endl
		 << "Usage: " << argv[0] << " [OPTION] --db DATABASE\n"
		 << "\t--url BASEURL [BASEDIRECTORY] DIRECTORY\n\n"
		 << "Index static website data via the filesystem.\n"
		 << "  -d, --duplicates\tset duplicate handling\n"
		 << "  \t\t\tone of `ignore', `replace', `duplicate'\n"
		 << "  -D, --db\t\tpath to database to use\n"
		 << "  -U, --url\t\tbase url DIRECTORY represents\n"
	         << "  -M, --mime-type\tadditional MIME mapping ext:type\n"
		 << "  -l, --no-recurse\tonly process the given directory\n"
		 << "  -f, --follow\t\tfollow symbolic links\n"
		 << "      --overwrite\tcreate the database anew (the default is\n"
	        "\t\t\tto update if the database already exists).\n"
		 << "  -h, --help\t\tdisplay this help and exit\n"
		 << "  -v, --version\t\toutput version and exit\n\n"
		 << "Report bugs via the web interface at:\n"
		 << "<http://xapian.org/bugs/>" << endl;
	    return 0;
	case 'v':
	    cout << OMINDEX << " (" << PACKAGE << ") " << VERSION << "\n"
		 << "Copyright (c) 1999,2000,2001 BrightStation PLC.\n"
		 << "Copyright (c) 2001 James Aylett\n"
		 << "Copyright (c) 2001,2002 Ananova Ltd\n"
		 << "Copyright (c) 2002,2003,2004 Olly Betts\n\n"
		 << "This is free software, and may be redistributed under\n"
		 << "the terms of the GNU Public License." << endl;
	    return 0;
	case 'd': // how shall we handle duplicate documents?
	    switch (optarg[0]) {
	    case 'd':
		dupes = DUPE_duplicate;
		break;
	    case 'i':
		dupes = DUPE_ignore;
		break;
	    case 'r':
		dupes = DUPE_replace;
		break;
	    }
	    break;
	case 'l': // Turn off recursion
	    recurse = 0;
	    break;
	case 'f': // Turn on following of symlinks
	    follow_symlinks = true;
	    break;
	case 'M': {
	    const char * s = strchr(optarg, ':');
	    if (s != NULL && s[1] != '\0') {
		mime_map[string(optarg, s - optarg)] = string(s + 1);
	    } else {
		cerr << "Illegal MIME mapping '" << optarg << "'\n"
		     << "Should be of the form ext:type, eg txt:text/plain"
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
	case ':': // missing param
	    return 1;
	case '?': // unknown option: FIXME -> char
	    return 1;
	}
    }

    if (dbpath.empty()) {
	cerr << OMINDEX << ": you must specify a database with --db.\n";
	return 1;
    }
    if (baseurl.empty()) {
	cerr << OMINDEX << ": you must specify a base URL with --url.\n";
	return 1;
    }
    // baseurl mustn't end '/' or you end up with the wrong URL
    // (//thing is different to /thing). We could probably make this
    // safe a different way, by ensuring that we don't put a leading '/'
    // on leafnames when scanning a directory, but this will do.
    if (baseurl[baseurl.length() - 1] == '/') {
	cout << "baseurl has trailing '/' ... removing ... " << endl;
	baseurl = baseurl.substr(0, baseurl.length()-1);
    }

    if (optind >= argc || optind + 2 < argc) {
	cerr << OMINDEX << ": you must specify a directory to index.\n"
"Do this either as a single directory (corresponding to the base URL)\n"
"or two directories - the first corresponding to the base URL and the second\n"
"a subdirectory of that to index." << endl;
	return 1;
    }
    root = argv[optind];
    if (optind + 2 == argc) {
	indexroot = argv[optind + 1]; // relative to root
	if (indexroot.empty() || indexroot[0] != '/') {
	    indexroot = "/" + indexroot;
	}
    } else {
	indexroot = ""; // index the whole of root
    }

    try {
	if (!overwrite) {
	    db = Xapian::WritableDatabase(dbpath, Xapian::DB_CREATE_OR_OPEN);
	    if (dupes == DUPE_replace) {
		// + 1 so that db.get_lastdocid() is a valid subscript.
		updated.resize(db.get_lastdocid() + 1);
	    }
	} else {
	    db = Xapian::WritableDatabase(dbpath, Xapian::DB_CREATE_OR_OVERWRITE);
	}
	index_directory("/", mime_map);
	if (dupes == DUPE_replace) {
	    for (Xapian::docid did = 1; did < updated.size(); ++did) {
		if (!updated[did]) {
		    try {
			db.delete_document(did);
			cout << "Deleted document #" << did << endl;
		    } catch (const Xapian::DocNotFoundError &) {
		    }
		}
	    }
	}
	db.flush();
	// cout << "\n\nNow we have " << db.get_doccount() << " documents.\n";
    } catch (const Xapian::Error &e) {
	cout << "Exception: " << e.get_msg() << endl;
	return 1;
    } catch (const string &s) {
	cout << "Exception: " << s << endl;
	return 1;
    } catch (const char *s) {
	cout << "Exception: " << s << endl;
	return 1;
    } catch (...) {
	cout << "Caught unknown exception" << endl;
	return 1;
    }
}
