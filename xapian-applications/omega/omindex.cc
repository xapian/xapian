/* omindex.cc: index static documents into the omega db
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 James Aylett
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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

#include <algorithm>
#include <string>
#include <map>
#include <memory>

#include <fstream>

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include <om/om.h>

#include "htmlparse.h"

using std::cout;
using std::cerr;
using std::endl;
using std::find;
using std::ifstream;
using std::auto_ptr;

#define OMINDEX "omindex"

#define DUPE_ignore 0
#define DUPE_replace 1
#define DUPE_duplicate 2
static int dupes = DUPE_replace;
static int recurse = 1;
static string dbpath;
static string root;
static string indexroot;
static string baseurl;
static OmWritableDatabase db;

// Put a limit on the size of terms to help prevent the index being bloated
// by useless junk terms
static const unsigned int MAX_PROB_TERM_LENGTH = 64;
static const unsigned int MAX_URL_LENGTH = 240;

static void
lowercase_term(om_termname &term)
{
    om_termname::iterator i = term.begin();
    while (i != term.end()) {
	*i = tolower(*i);
	i++;
    }
}

class MyHtmlParser : public HtmlParser {
    public:
	bool in_script_tag;
	bool in_style_tag;
    	string title, sample, keywords, dump;
	bool indexing_allowed;
	void process_text(const string &text);
	void opening_tag(const string &tag, const map<string,string> &p);
	void closing_tag(const string &tag);
	MyHtmlParser() :
		in_script_tag(false),
		in_style_tag(false),
		indexing_allowed(true) { }
};

void
MyHtmlParser::process_text(const string &text)
{
    // some tags are meaningful mid-word so this is simplistic at best...

    if (!in_script_tag && !in_style_tag) {
	string::size_type firstchar = text.find_first_not_of(" \t\n\r");
	if (firstchar != string::npos) {
	    dump += text.substr(firstchar);
	    dump += " ";
	}
    }
}

void
MyHtmlParser::opening_tag(const string &tag, const map<string,string> &p)
{
#if 0
    cout << "<" << tag;
    map<string, string>::const_iterator x;
    for (x = p.begin(); x != p.end(); x++) {
	cout << " " << x->first << "=\"" << x->second << "\"";
    }
    cout << ">\n";
#endif
    
    if (tag == "meta") {
	map<string, string>::const_iterator i, j;
	if ((i = p.find("content")) != p.end()) {
	    if ((j = p.find("name")) != p.end()) {
		string name = j->second;
		lowercase_term(name);
		if (name == "description") {
		    if (sample.empty()) {
			sample = i->second;
			decode_entities(sample);
		    }
		} else if (name == "keywords") {
		    if (!keywords.empty()) keywords += ' ';
		    string tmp = i->second;
		    decode_entities(tmp);
		    keywords += tmp;
		} else if (name == "robots") {
		    string val = i->second;
		    decode_entities(val);
		    lowercase_term(val);
		    if (val.find("none") != string::npos ||
			val.find("noindex") != string::npos) {
			indexing_allowed = false;
		    }
		}
	    }
	}
    } else if (tag == "script") {
	in_script_tag = true;
    } else if (tag == "style") {
	in_style_tag = true;
    }
}

void
MyHtmlParser::closing_tag(const string &text)
{
    string x = text;
    if (x == "title") {
	title = dump;
	// replace newlines with spaces
	size_t i = 0;    
	while ((i = title.find("\n", i)) != string::npos) title[i] = ' ';
	dump = "";
    } else if (x == "script") {
	in_script_tag = false;
    } else if (x == "style") {
	in_style_tag = false;
    }
}

#if 0
inline static bool
p_alpha(unsigned int c)
{
    return ((c | 32) - 'a') <= ('z' - 'a');
}
#endif

inline static bool
p_alnum(unsigned int c)
{
    return isalnum(c);
}

inline static bool
p_notalnum(unsigned int c)
{
    return !isalnum(c);
}

inline static bool
p_notplusminus(unsigned int c)
{
    return c != '+' && c != '-';
}

static om_termpos
index_text(const string &s, OmDocument &doc, OmStem &stemmer, om_termpos pos)
{
    string::const_iterator i, j = s.begin(), k;
    while ((i = find_if(j, s.end(), p_alnum)) != s.end()) {
	om_termname term;
	k = i;
	if (isupper(*k)) {
	    j = k;
	    term = *j;
	    while (++j != s.end() && *j == '.' &&
		   ++j != s.end() && isupper(*j)) {
		term += *j;
	    } 
	    if (term.length() < 2 || (j != s.end() && isalnum(*j))) {
		term = "";
	    }
	}
	if (term.empty()) {
moreterm:
	    j = find_if(k, s.end(), p_notalnum);
	    if (j != s.end() && *j == '&') {
		if (j + 1 != s.end() && isalnum(j[1])) {
		    k = j + 1;
		    goto moreterm;
		}
	    }
	    k = find_if(j, s.end(), p_notplusminus);
	    if (k == s.end() || !isalnum(*k)) j = k;
	    term = s.substr(i - s.begin(), j - i);
	}
	if (term.length() <= MAX_PROB_TERM_LENGTH) {
	    lowercase_term(term);
	    if (isupper(*i) || isdigit(*i)) {
		doc.add_posting('R' + term, pos);
	    }

	    term = stemmer.stem_word(term);
	    doc.add_posting(term, pos++);
	}
    }
    return pos;
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
hash(const string &s)
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
    if (1 + baseurl.length() + url.length() > MAX_URL_LENGTH) {
	string result = "U" + baseurl + url;
	result = result.substr(0, MAX_URL_LENGTH - HASH_LEN) +
		hash(result.substr(MAX_URL_LENGTH - HASH_LEN));
	//printf("Using '%s' as the url term\n", result.c_str());
	return result;
    }
    return "U" + baseurl + url;
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

	string::size_type space = output.find_last_of(" \t\n\r");
	space = 0;
	if (space != string::npos && space > maxlen / 2) {
	    string::size_type nonspace;
	    nonspace = output.find_last_not_of(" \t\n\r", space);
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
	ifstream in(file.c_str());
	if (!in) {
	    cout << "can't open \"" << file << "\" - skipping\n";
	    return;
	}
	string text;
	while (!in.eof()) {
	    string line;
	    getline(in, line);
	    text += line + '\n';
	}
	in.close();
	MyHtmlParser p;
	p.parse_html(text);
	if (!p.indexing_allowed) {
	    cout << "indexing disallowed by meta tag\n";
	    return;
	}
	dump = p.dump;
	title = p.title;
	keywords = p.keywords;
	sample = p.sample;
    } else if (mimetype == "text/plain") {
	ifstream in(file.c_str());
	if (!in) {
	    cout << "can't open \"" << file << "\" - skipping\n";
	    return;
	}
	while (!in.eof()) {
	    string line;
	    getline(in, line);
	    dump += line + '\n';
	}
	in.close();	
    } else if (mimetype == "application/pdf") {
	string safefile = file;
	string::size_type p = 0;
	while (p < safefile.size()) {
	    if (!isalnum(safefile[p])) safefile.insert(p++, "\\");
            p++;
	}
	string tmp = "/tmp/omindex.txt";
	unlink(tmp.c_str());
	string cmd = "pdftotext " + safefile + " " + tmp;
	if (system(cmd.c_str()) != 0) {
	    cout << "pdftotext failed to extract text from \"" << file << "\" - skipping\n";
	    return;
	}
	ifstream in(tmp.c_str());
	unlink(tmp.c_str());
	if (!in) {
	    cout << "pdftotext failed to extract text from \"" << file << "\" - skipping\n";
	    return;
	}
	while (!in.eof()) {
	    string line;
	    getline(in, line);
	    dump += line + '\n';
	}
	in.close();
    } else if (mimetype == "application/postscript") {
	string safefile = file;
	string::size_type p = 0;
	while (p < safefile.size()) {
	    if (!isalnum(safefile[p])) safefile.insert(p++, "\\");
            p++;
	}
	string tmp = "/tmp/omindex.txt";
	unlink(tmp.c_str());
	string cmd = "pstotext -output " + tmp + ' ' + safefile;
	if (system(cmd.c_str()) != 0) {
	    cout << "pstotext failed to extract text from \"" << file << "\" - skipping\n";
	    return;
	}
	ifstream in(tmp.c_str());
	unlink(tmp.c_str());
	if (!in) {
	    cout << "pstotext failed to extract text from \"" << file << "\" - skipping\n";
	    return;
	}
	while (!in.eof()) {
	    string line;
	    getline(in, line);
	    dump += line + '\n';
	}
	in.close();
    } else {
	// Don't know how to index this
	cout << "unknown MIME type - skipping\n";
	return;
    }
    OmStem stemmer("english");    

    // Produce a sample
    if (sample.empty()) {
	sample = truncate_to_word(dump, 300);
    } else {
	sample = truncate_to_word(sample, 300);
    }

    // Put the data in the document
    OmDocument newdocument;
    string record = "url=" + baseurl + url + "\nsample=" + sample;
    if (!title.empty()) {
	record = record + "\ncaption=" + truncate_to_word(title, 100);
    }
    record = record + "\ntype=" + mimetype;
    newdocument.set_data(record);

    // Add postings for terms to the document
    om_termpos pos = 1;
    pos = index_text(title, newdocument, stemmer, pos);
    pos = index_text(dump, newdocument, stemmer, pos + 100);
    pos = index_text(keywords, newdocument, stemmer, pos + 100);

    newdocument.add_term_nopos("T" + mimetype); // mimeType
    string::size_type j;
    j = find_if(baseurl.begin(), baseurl.end(), p_notalnum) - baseurl.begin();
    if (j > 0 && baseurl.substr(j, 3) == "://") {
	j += 3;
    	string::size_type k = baseurl.find('/', j);
	if (k == string::npos) {
	  newdocument.add_term_nopos("P/"); // Path
	  newdocument.add_term_nopos("H" + baseurl.substr(j));
	} else {
	  newdocument.add_term_nopos("P" + baseurl.substr(k)); // Path
	  string::const_iterator l = find(baseurl.begin() + j, baseurl.begin() + k, ':');
	  newdocument.add_term_nopos("H" + baseurl.substr(j, l - baseurl.begin() - j)); // Host
	}
    } else {
	newdocument.add_term_nopos("P" + baseurl); // Path
    }

    struct tm *tm = localtime(&last_mod);
    char buf[9];
    sprintf(buf, "%04d%02d%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    newdocument.add_term_nopos("D" + string(buf)); // Date (YYYYMMDD)
    buf[7] = '\0';
    if (buf[6] == '3') buf[6] = '2';
    newdocument.add_term_nopos("W" + string(buf)); // "Weak" - 10ish day interval
    buf[6] = '\0';
    newdocument.add_term_nopos("M" + string(buf)); // Month (YYYYMM)
    buf[4] = '\0';
    newdocument.add_term_nopos("Y" + string(buf)); // Year (YYYY)
    newdocument.add_term_nopos(urlterm); // Url

    if (dupes == DUPE_replace && db.term_exists(urlterm)) {
	// This document has already been indexed - update!
	try {
	    auto_ptr<OmEnquire> enq = auto_ptr<OmEnquire>(new OmEnquire(db));
	    enq->set_query(OmQuery(urlterm));
	    OmMSet mset = enq->get_mset(0, 1);
	    try {
		db.replace_document(*mset[0], newdocument);
	    } catch (...) {
	    }
	} catch (...) {
	    db.add_document(newdocument);
	    cout << "(failed re-seek) ";
	}
	cout << "duplicate. Re-indexed." << endl;
    } else {
	db.add_document(newdocument);
	cout << "done." << endl;
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
	if (stat(file.c_str(), &statbuf) == -1) {
	    cout << "Can't stat \"" << file << "\" - skipping\n";
	    continue;
	}
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
    // getopt
    char* optstring = "hvd:D:U:M:l";
    struct option longopts[8] = {
	{ "help",	0,			NULL, 'h' },
	{ "version",	0,			NULL, 'v' },
	{ "duplicates",	required_argument,	NULL, 'd' },
	{ "db",		required_argument,	NULL, 'D' },
	{ "url",	required_argument,	NULL, 'U' },
	{ "mime-type",	required_argument,	NULL, 'M' },
	{ "no-recurse",	0,			NULL, 'l' },
	{ 0, 0, NULL, 0 }
    };

    int longindex, getopt_ret;

    map<string, string> mime_map = map<string, string>();
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

    while ((getopt_ret = getopt_long(argc, argv, optstring,
				     longopts, &longindex))!=EOF) {
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
		 << "Copyright (c) 2002,2003 Olly Betts\n\n"
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

    if (optind >= argc || optind+2 < argc) {
	cerr << OMINDEX << ": you must specify a directory to index.\nDo this either as a single directory (taken to be the store of the base URL)\nor two, one the store of the base URL and one a dir within that to index.";
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
	db = OmWritableDatabase(OmAuto__open(dbpath, OM_DB_CREATE_OR_OPEN));
	index_directory("/", mime_map);
	// db.reopen(); // Ensure we're up to date
	// cout << "\n\nNow we have " << db.get_doccount() << " documents.\n";
	db.flush();
    } catch (const OmError &e) {
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
