/* scriptindex.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Sam Liddicott
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

#include <xapian.h>

#include <algorithm>
#include <fstream>
#include <iostream>
// Not in GCC 2.95.2: #include <limits>
#include <map>
#include <string>
#include <vector>
#include <list>

#include <getopt.h>
#include <stdio.h>
#include <unistd.h>

#include "htmlparse.h"
#include "indextext.h"

using namespace std;

#ifdef __WIN32__
inline unsigned int sleep(unsigned int secs) {
    _sleep(secs * 1000);
    return 0;
}
#endif

static const char *argv0;
static bool verbose;
static int addcount;
static int repcount;
static int delcount;

// Put a limit on the size of terms to help prevent the index being bloated
// by useless junk terms
static const unsigned int MAX_PROB_TERM_LENGTH = 64;

class MyHtmlParser : public HtmlParser {
    public:
    	string dump;
	void process_text(const string &text) {
	    // some tags are meaningful mid-word so this is a little
	    // simplistic
	    if (!dump.empty()) dump += ' ';
	    dump += text;
	}
	void closing_tag(const string &tag) {
	    if (tag == "title") dump = "";
	}
};

inline static bool 
p_space(unsigned int c)
{
    return isspace(c);
}

inline static bool 
p_notspace(unsigned int c)
{
    return !isspace(c);
}

inline static bool
p_notalpha(unsigned int c)
{
    return !isalpha(c);
}

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

// Characters allowed as second or subsequent characters in a fieldname
inline static bool
p_notfieldnamechar(unsigned int c)
{
    return !isalnum(c) && c != '_';
}

class Action {
public:
    typedef enum {
	BAD, NEW,
	BOOLEAN, DATE, FIELD, INDEX, INDEXNOPOS, LOWER,
	TRUNCATE, UNHTML, UNIQUE, VALUE, WEIGHT
    } type;
private:
    type action;
    int num_arg;
    string string_arg;
public:
    Action(type action_, string arg = "") : action(action_), string_arg(arg) {
	num_arg = atoi(string_arg.c_str());
    }
    type get_action() const { return action; }
    int get_num_arg() const { return num_arg; }
    string get_string_arg() const { return string_arg; }
};

static map<string, vector<Action> > index_spec;

static void
parse_index_script(const string &filename)
{
    ifstream script(filename.c_str());
    string line;
    while (getline(script, line)) {
	vector<string> fields;
	vector<Action> actions;
	string::const_iterator i, j;
	const string &s = line;
	i = find_if(s.begin(), s.end(), p_notspace);
	if (i == s.end() || *i == '#') continue;
	while (true) {
	    if (!isalnum(*i)) {
		cout << argv0 << ": field name must start with alphanumeric"
		     << endl;
		exit(1);
	    }
	    j = find_if(i, s.end(), p_notfieldnamechar);
	    fields.push_back(string(i, j));
	    i = find_if(j, s.end(), p_notspace);
	    if (i == s.end()) break;
	    if (*i == ':') {
		++i;
		i = find_if(i, s.end(), p_notspace);
		break;
	    }
	    if (i == j) {
		cout << argv0 << ": bad character '" << *j << "' in fieldname"
		     << endl;
		exit(1);
	    }
	}
	j = i;
	while (j != s.end()) {
	    i = find_if(j, s.end(), p_notalpha);
	    string action = s.substr(j - s.begin(), i - j);
	    Action::type code = Action::BAD;
	    enum {NO, OPT, YES} arg = NO;
	    if (!action.empty()) {
		switch (action[0]) {
		    case 'b':
			if (action == "boolean") {
			    code = Action::BOOLEAN;  
			    arg = OPT;
			}
			break;
		    case 'd':
			if (action == "date") {
			    code = Action::DATE;  
			    arg = YES;
			}
			break;
		    case 'f':
			if (action == "field") {
			    code = Action::FIELD;  
			    arg = OPT;
			}
			break;
		    case 'i':
			if (action == "index") {
			    code = Action::INDEX;
			    arg = OPT;
			} else if (action == "indexnopos") {
			    code = Action::INDEXNOPOS;
			    arg = OPT;
			}
			break;
		    case 'l':
			if (action == "lower") {
			    code = Action::LOWER;  
			}
			break;
		    case 't':
			if (action == "truncate") {
			    code = Action::TRUNCATE;  
			    arg = YES;
			}
			break;
		    case 'u':
			if (action == "unhtml") {
			    code = Action::UNHTML;  
			} else if (action == "unique") {
			    code = Action::UNIQUE;  
			    arg = YES;
			}
			break;
		    case 'v':
			if (action == "value") {
			    code = Action::VALUE;  
			    arg = YES;
			}
			break;
		    case 'w':
			if (action == "weight") {
			    code = Action::WEIGHT;  
			    arg = YES;
			}
			break;
		}
	    }
	    if (code == Action::BAD) {
		cout << argv0 << ": unknown index action `" << action
		     << "'" << endl;
		exit(1);
	    }
	    i = find_if(i, s.end(), p_notspace);

	    if (i != s.end() && *i == '=') {
		if (arg == NO) {
		    cout << argv0 << ": index action `" << action
			 << "' doesn't take an argument" << endl;
		    exit(1);
		}
		++i;
		j = find_if(i, s.end(), p_notspace);
		i = find_if(j, s.end(), p_space);
		string arg = string(j, i);
		if (code == Action::INDEX && arg == "nopos") {
		    // index used to take an optional argument which could
		    // be "nopos" to mean the same that indexnopos now does.
		    // translate this to allow older scripts to work (this
		    // is safe to do since nopos isn't a sane prefix value)
		    actions.push_back(Action(Action::INDEXNOPOS));
		} else {
		    actions.push_back(Action(code, arg));
		}
		i = find_if(i, s.end(), p_notspace);
	    } else {
		if (arg == YES) {
		    cout << argv0 << ": index action `" << action
			 << "' must have an argument" << endl;
		    exit(1);
		}
		actions.push_back(Action(code));
	    }
	    j = i;
	}
	vector<string>::const_iterator field;
	for (field = fields.begin(); field != fields.end(); ++field) {
	    vector<Action> &v = index_spec[*field];
	    if (v.empty()) {
		v = actions;
	    } else {
		v.push_back(Action(Action::NEW));
		v.insert(v.end(), actions.begin(), actions.end());
	    }
	}
    }
}

static void
lowercase_term(string &term)
{
    string::iterator i = term.begin();
    while (i != term.end()) {
        *i = tolower(*i);
        i++;
    }
} 

static void
lowercase_string(string &term)
{
    string::iterator i = term.begin();
    while (i != term.end()) {
        *i = tolower(*i);
        i++;
    }
} 

static bool
index_file(istream &stream, Xapian::WritableDatabase &database,
	   Xapian::Stem &stemmer)
{
    string line;
    if (!getline(stream, line)) {
	// empty file !?!
	return true;
    }
    
    while (true) {
	Xapian::Document doc;
	Xapian::docid docid = 0;
	Xapian::termpos wordcount = 0;
	map<string, list<string> > fields;
	bool seen_content = false;
	while (true) {
	    Xapian::termcount weight = 1;
	    // Cope with files from MS Windows (\r\n end of lines)
	    if (line[line.size() - 1] == '\r') line.resize(line.size() - 1);

	    string::size_type eq = line.find('=');
	    string field = line.substr(0, eq);
	    string value = line.substr(eq + 1);
	    while (getline(stream, line) && !line.empty() && line[0] == '=') {
		// Cope with files from MS Windows (\r\n end of lines)
		if (line[line.size() - 1] == '\r') line.resize(line.size() - 1);
		value += '\n' + line.substr(1);
	    }

	    vector<Action> &v = index_spec[field];
	    string old_value = value;
	    vector<Action>::const_iterator i;
	    bool this_field_is_content = true;
	    for (i = v.begin(); i != v.end(); ++i) {
		switch (i->get_action()) {
		    case Action::BAD:
			abort();
		    case Action::NEW:
			value = old_value;
			break;
		    case Action::FIELD:
			if (!value.empty()) {
			    string f = i->get_string_arg();
			    if (f.empty()) f = field;
			    // replace newlines with spaces
			    string s = value;
			    string::size_type i = 0;
			    while ((i = s.find('\n', i)) != string::npos)
				s[i] = ' ';
			    fields[f].push_back(s);
			}
			break;
		    case Action::INDEX:
			wordcount = index_text(value, doc, stemmer, weight,
					       i->get_string_arg(), wordcount);
			break;
		    case Action::INDEXNOPOS:
			// No positional information so phrase searching
			// won't work.  However, the database will use much
			// less diskspace.
			index_text(value, doc, stemmer, weight,
				   i->get_string_arg());	
			break;
		    case Action::BOOLEAN:
			doc.add_term(i->get_string_arg() + value);
			break;
		    case Action::LOWER:
			lowercase_string(value); 
			break;
		    case Action::TRUNCATE: {
			string::size_type l = i->get_num_arg();
			if (l < value.size()) {
			    while (l > 0 && !isspace(value[l - 1])) --l;
			    while (l > 0 && isspace(value[l - 1])) --l;

			    // If the first word is too long, just truncate it
			    if (l == 0) l = i->get_num_arg();

			    value = value.substr(0, l);
			}
			break;
		    }
		    case Action::UNHTML: {
			MyHtmlParser p;
			p.parse_html(value);
			value = p.dump;
			break;
		    }
		    case Action::UNIQUE: {
			// Ensure that the value of this field is unique.
			// If a record already exists with the same value,
			// it will be replaced with the new record.

			// Unique fields aren't considered content - if
			// there are no other fields in the document, the
			// document is to be deleted.
			this_field_is_content = false;

			// Argument is the prefix to add to the field value
			// to get the unique term.
			string t = i->get_string_arg();
			t += value;
again:
			try {
			    Xapian::PostingIterator p = database.postlist_begin(t);
			    if (p != database.postlist_end(t)) {
				docid = *p;
			    }
			} catch (const Xapian::Error &e) {
			    // Hmm, what happened?
			    cout << "Caught exception in UNIQUE!" << endl;
			    cout << "E: " << e.get_msg() << endl;
			    database.flush();
			    database.reopen();
			    goto again;
			}
			break;
		    }
		    case Action::VALUE:
			if (!value.empty())
			    doc.add_value(i->get_num_arg(), value);
			break;
		    case Action::WEIGHT:
			weight = i->get_num_arg();
			break;
		    case Action::DATE: {
			string type = i->get_string_arg();
			switch (type[0]) {
			    case 'u':
				if (type == "unix") {
				    char buf[9];
				    struct tm *tm;
				    time_t t = atoi(value.c_str());
				    tm = localtime(&t);
				    sprintf(buf, "%04d%02d%02d",
					    tm->tm_year + 1900, tm->tm_mon + 1,
					    tm->tm_mday);
				    value = buf;
				    break;
				}
				value = "";
				break;
			    case 'y':
				if (type == "yyyymmdd") {
				    if (value.length() == 8) break;
				}
				value = "";
				break;
			    default:
				value = "";
				break;
			}
			if (value.empty()) break;
			// Date (YYYYMMDD)
			doc.add_term("D" + value);
#if 0 // "Weak" terms aren't currently used by omega
			value.resize(7);
			if (value[6] == '3') value[6] = '2';
			// "Weak" - 10ish day interval
			newdocument.add_term("W" + value);
#endif
			value.resize(6);
			// Month (YYYYMM)
			doc.add_term("M" + value);
			value.resize(4);
			// Year (YYYY)
			doc.add_term("Y" + value);
			break;
		    }
		}
	    }
	    if (this_field_is_content) seen_content = true;
	    if (line.empty() || line == "\r") break;
	}

	// If we haven't seen any fields (other than unique identifiers)
	// the document is to be deleted.
	if (!seen_content) {
	    if (docid) {
		database.delete_document(docid);
		if (verbose) cout << "Del: " << docid << endl;
		delcount ++;
	    }
	} else {
	    string data;
	    map<string, list<string> >::const_iterator i;
	    for (i = fields.begin(); i != fields.end(); ++i) {
		list<string>::const_iterator j;
		for (j = i->second.begin(); j != i->second.end(); j++) {
		    data += i->first;
		    data += '=';
		    data += *j;
		    data += '\n';
		}
	    }

	    // Put the data in the document
	    doc.set_data(data);

	    // Add the document to the database
	    if (docid) {
		try {
		    database.replace_document(docid, doc);
		    if (verbose) cout << "Replace: " << docid << endl;
		    repcount ++;
		} catch (const Xapian::Error &e) {
		    cout << "E: " << e.get_msg() << endl;
		    // Possibly the document was deleted by another
		    // process in the meantime...?
		    docid = database.add_document(doc);
		    cout << "Replace failed, adding as new: " << docid << endl;
		}
	    } else {
		docid = database.add_document(doc);
		if (verbose) cout << "Add: " << docid << endl;
		addcount ++;
	    }
	}

	if (stream.eof() || !getline(stream, line)) break;
    }

    //cout << "Flushing: " << endl;
    //database.flush();

    return true;
}

int
main(int argc, char **argv)
{
    // If overwrite is true, the database will be created anew even if it
    // already exists.
    bool overwrite = false;
    bool quiet = false;
    verbose = false;

    argv0 = argv[0];
    static const struct option longopts[] = {
	{ "help",	no_argument,	NULL, 'h' },
	{ "version",	no_argument,	NULL, 'V' },
	{ "overwrite",	no_argument,	NULL, 'o' },
	{ "quiet",	no_argument,	NULL, 'q' },
	{ "verbose",	no_argument,	NULL, 'v' },
	{ 0, 0, NULL, 0 }
    };

    bool more = true, show_help = false;
    while (more) {
	switch (getopt_long(argc, argv, "uqv", longopts, NULL)) {
	    case EOF:
		more = false;
		break;
	    default:
	    case 'h': // --help
	    case 'V': // --version
		show_help = true;
		more = false;
		break;
	    case 'o': // --overwrite
		overwrite = true;
		break;
	    case 'u':
		// Update is now the default, so ignore -u for compatibility...
		break;
	    case 'q':
		quiet = true;
		break;
	    case 'v':
		verbose = true;
		break;
	}
    }

    argv += optind;
    argc -= optind;
    if (show_help || argc < 2) {
	cout << "Usage: " << argv0
	     << " [--help] [--version] [--overwrite] [-q] [-v] "
	     << " <path to xapian database> <indexer script> [<filename>]..."
	     << endl
	     << "Creates a new database containing the data given by the list "
	     << "of files."
	     << endl
	     << "The -q (quiet) option suppresses log messages."
	     << endl
	     << "The -v (verbose) option generates messages about all actions."
	     << endl
	     << "The --overwrite option causes the database to be created anew (the default is\n"
	        "to update if the database already exists)."
	     << endl;
	exit(1);
    }
    
    parse_index_script(argv[1]);
    
    // Catch any Xapian::Error exceptions thrown
    try {
	// Make the database
	// Sleep and retry if we get an Xapian::DatabaseLockError - this just
	// means that another process is updating the database
	Xapian::WritableDatabase database;
	while (true) {
	    try {
		if (!overwrite) {
		    database = Xapian::Auto::open(argv[0],
						  Xapian::DB_CREATE_OR_OPEN);
		} else {
		    database = Xapian::Auto::open(argv[0],
						  Xapian::DB_CREATE_OR_OVERWRITE);
		}
		break;
	    } catch (const Xapian::DatabaseLockError &error) {
		cout << "Database locked ... retrying" << endl;
		sleep(1);
	    }
	}
	Xapian::Stem stemmer("english"); 

	addcount = 0;
	repcount = 0;
	delcount = 0;

	// Read file/s
	if (argc == 2) {
	    // Read from stdin
	    index_file(cin, database, stemmer);
	} else {
	    for (int i = 2; i < argc; ++i) {
		ifstream stream(argv[i]);
		if (stream) {
		    index_file(stream, database, stemmer);
		} else {
		    cout << "Can't open file " << argv[i] << endl;
		}
	    }
	}

	if (verbose) cout << "Flushing: " << endl;
	database.flush();

	cout << "records (added, replaced, deleted) = (" << addcount <<
		", " << repcount << ", " << delcount << ")" << endl;
    } catch (const Xapian::Error &error) {
	cout << "Exception: " << error.get_msg() << endl;
	exit(1);
    } catch (const std::bad_alloc &) {
	cout << "Exception: std::bad_alloc" << endl;
	exit(1);
    } catch (...) {
	cout << "Unknown Exception" << endl;
	exit(1);
    }
}
