/* scriptindex.cc
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Sam Liddicott
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006 Olly Betts
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

#include <xapian.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "commonhelp.h"
#include "hashterm.h"
#include "indextext.h"
#include "loadfile.h"
#include "myhtmlparse.h"
#include "utils.h"

#include "gnu_getopt.h"

using namespace std;

#define PROG_NAME "scriptindex"
#define PROG_DESC "index arbitrary data as described by an index script"

#ifdef __WIN32__
inline unsigned int sleep(unsigned int secs) {
    _sleep(secs * 1000);
    return 0;
}
#endif

static bool verbose;
static int addcount;
static int repcount;
static int delcount;

inline static bool
p_space(unsigned int c)
{
    return isspace(static_cast<unsigned char>(c));
}

inline static bool
p_notspace(unsigned int c)
{
    return !isspace(static_cast<unsigned char>(c));
}

inline static bool
p_notalpha(unsigned int c)
{
    return !isalpha(static_cast<unsigned char>(c));
}

// Characters allowed as second or subsequent characters in a fieldname
inline static bool
p_notfieldnamechar(unsigned int c)
{
    return !isalnum(static_cast<unsigned char>(c)) && c != '_';
}

const char * action_names[] = {
    "bad", "new",
    "boolean", "date", "field", "hash", "index", "indexnopos", "load", "lower",
    "truncate", "unhtml", "unique", "value", "weight"
};

// For debugging:
#define DUMP_ACTION(A) cout << action_names[(A).get_action()] << "(" << (A).get_string_arg() << "," << (A).get_num_arg() << ")" << endl

class Action {
public:
    typedef enum {
	BAD, NEW,
	BOOLEAN, DATE, FIELD, HASH, INDEX, INDEXNOPOS, LOAD, LOWER,
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
    Action(type action_, string arg, int num)
	: action(action_), num_arg(num), string_arg(arg) { }
    type get_action() const { return action; }
    int get_num_arg() const { return num_arg; }
    const string & get_string_arg() const { return string_arg; }
};

static void
report_useless_action(const string &file, size_t line, size_t pos,
		      const string &action)
{
    cout << file << ':' << line;
    if (pos != string::npos) cout << ':' << pos;
    cout << ": Warning: Index action '" << action << "' has no effect" << endl;

    static bool given_left_to_right_warning = false;
    if (!given_left_to_right_warning) {
	given_left_to_right_warning = true;
	cout << file << ':' << line
	     << ": Warning: Note that actions are executed from left to right"
	     << endl;
    }
}

static map<string, vector<Action> > index_spec;

static void
parse_index_script(const string &filename)
{
    ifstream script(filename.c_str());
    if (!script.is_open()) {
	cout << filename << ": " << strerror(errno) << endl;
	exit(1);
    }
    string line;
    size_t line_no = 0;
    while (getline(script, line)) {
	++line_no;
	vector<string> fields;
	vector<Action> actions;
	string::const_iterator i, j;
	const string &s = line;
	i = find_if(s.begin(), s.end(), p_notspace);
	if (i == s.end() || *i == '#') continue;
	while (true) {
	    if (!isalnum(static_cast<unsigned char>(*i))) {
		cout << filename << ':' << line_no
		     << ": field name must start with alphanumeric" << endl;
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
		cout << filename << ':' << line_no
		     << ": bad character '" << *j << "' in fieldname" << endl;
		exit(1);
	    }
	}
	Xapian::termcount weight = 1;
	size_t useless_weight_pos = string::npos;
	map<string, Action::type> boolmap;
	j = i;
	while (j != s.end()) {
	    i = find_if(j, s.end(), p_notalpha);
	    string action = s.substr(j - s.begin(), i - j);
	    Action::type code = Action::BAD;
	    enum {NO, OPT, YES} arg = NO;
	    bool takes_integer_argument = false;
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
		    case 'h':
			if (action == "hash") {
			    code = Action::HASH;
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
			} else if (action == "load") {
			    code = Action::LOAD;
			}
			break;
		    case 't':
			if (action == "truncate") {
			    code = Action::TRUNCATE;
			    arg = YES;
			    takes_integer_argument = true;
			    if (!actions.empty() &&
				actions.back().get_action() == Action::LOAD) {
				/* Turn "load truncate=n" into "load" with
				 * num_arg n, so that we don't needlessly
				 * allocate memory and read data we're just
				 * going to ignore.
				 */
				actions.pop_back();
				code = Action::TRUNCATE;
			    }
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
			    takes_integer_argument = true;
			}
			break;
		    case 'w':
			if (action == "weight") {
			    code = Action::WEIGHT;
			    arg = YES;
			    takes_integer_argument = true;
			}
			break;
		}
	    }
	    if (code == Action::BAD) {
		cout << filename << ':' << line_no
		     << ": Unknown index action '" << action << "'" << endl;
		exit(1);
	    }
	    i = find_if(i, s.end(), p_notspace);

	    if (i != s.end() && *i == '=') {
		if (arg == NO) {
		    cout << filename << ':' << line_no
			 << ": Index action '" << action
			 << "' doesn't take an argument" << endl;
		    exit(1);
		}
		++i;
		j = find_if(i, s.end(), p_notspace);
		i = find_if(j, s.end(), p_space);
		string arg = string(j, i);
		if (takes_integer_argument) {
		    if (arg.find('.') != string::npos) {
			cout << filename << ':' << line_no
			     << ": Warning: Index action '" << action
			     << "' takes an integer argument" << endl;
		    }
		}
		switch (code) {
		    case Action::INDEX:
			if (arg == "nopos") {
			    // INDEX used to take an optional argument which
			    // could be "nopos" to mean the same that
			    // INDEXNOPOS now does.
			    cout << filename << ':' << line_no
				 << ": Warning: Index action '" << action
				 << '=' << arg << "' is deprecated - "
				    "use action 'indexnopos' instead" << endl;
			    // Translate this to allow older scripts to work
			    // (this is safe to do since nopos isn't a sane
			    // prefix value).
			    code = Action::INDEXNOPOS;
			    arg = "";
			}
			/* FALLTHRU */
		    case Action::INDEXNOPOS:
			actions.push_back(Action(code, arg, weight));
			useless_weight_pos = string::npos;
			break;
		    case Action::WEIGHT:
			// We don't push an Action for WEIGHT - instead we
			// store it ready to use in the INDEX and INDEXNOPOS
			// Actions.
			weight = atoi(arg.c_str());
			if (useless_weight_pos != string::npos) {
			    report_useless_action(filename, line_no,
						  useless_weight_pos, action);
			}
			useless_weight_pos = j - s.begin();
			break;
		    case Action::UNIQUE:
			if (boolmap.find(arg) == boolmap.end())
			    boolmap[arg] = Action::UNIQUE;
			actions.push_back(Action(code, arg));
			break;
		    case Action::BOOLEAN:
			boolmap[arg] = Action::BOOLEAN;
			/* FALLTHRU */
		    default:
			actions.push_back(Action(code, arg));
		}
		i = find_if(i, s.end(), p_notspace);
	    } else {
		if (arg == YES) {
		    cout << filename << ':' << line_no
			 << ": Index action '" << action
			 << "' must have an argument" << endl;
		    exit(1);
		}
		if (code == Action::INDEX || code == Action::INDEXNOPOS) {
		    useless_weight_pos = string::npos;
		    actions.push_back(Action(code, "", weight));
		} else {
		    actions.push_back(Action(code));
		}
	    }
	    j = i;
	}

	if (useless_weight_pos != string::npos) {
	    report_useless_action(filename, line_no, useless_weight_pos,
				  "weight");
	}

	while (!actions.empty()) {
	    bool done = true;
	    Action::type action = actions.back().get_action();
	    switch (action) {
		case Action::HASH:
		case Action::LOWER:
		case Action::TRUNCATE:
		case Action::UNHTML:
		    done = false;
		    report_useless_action(filename, line_no, string::npos,
					  action_names[action]);
		    actions.pop_back();
		    break;
		default:
		    break;
	    }
	    if (done) break;
	}

	map<string, Action::type>::const_iterator boolpfx;
	for (boolpfx = boolmap.begin(); boolpfx != boolmap.end(); ++boolpfx) {
	    if (boolpfx->second == Action::UNIQUE) {
		cout << filename << ':' << line_no
		     << ": Warning: Index action 'unique=" << boolpfx->first
		     << "' without 'boolean=" << boolpfx->first << "'" << endl;
		static bool given_doesnt_imply_boolean_warning = false;
		if (!given_doesnt_imply_boolean_warning) {
		    given_doesnt_imply_boolean_warning = true;
		    cout << filename << ':' << line_no
			 << ": Warning: Note 'unique' doesn't implicitly add "
			    "a boolean term" << endl;
		}
	    }
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

static bool
index_file(const char *fname, istream &stream,
	   Xapian::WritableDatabase &database, Xapian::Stem &stemmer)
{
    string line;
    size_t line_no = 0;
    while (!stream.eof() && getline(stream, line)) {
	++line_no;
	Xapian::Document doc;
	Xapian::docid docid = 0;
	Xapian::termpos wordcount = 0;
	map<string, list<string> > fields;
	bool seen_content = false;
	while (!line.empty()) {
	    // Cope with files from MS Windows (\r\n end of lines).
	    // Trim multiple \r characters, since that seems the best way
	    // to handle that case.
	    string::size_type last = line.find_last_not_of('\r');
	    if (last == string::npos) break;
	    line.resize(last + 1);

	    string::size_type eq = line.find('=');
	    if (eq == string::npos && !line.empty()) {
		cout << fname << ':' << line_no << ": expected = somewhere "
		    "in this line" << endl;
		// FIXME: die or what?
	    }
	    string field = line.substr(0, eq);
	    string value = line.substr(eq + 1);
	    while (getline(stream, line) && !line.empty() && line[0] == '=') {
		// Cope with files from MS Windows (\r\n end of lines).
		// Trim multiple \r characters, since that seems the best way
		// to handle that case.
		last = line.find_last_not_of('\r');
		// Since line[0] == '=', so last != string::npos.
		// Replace the = with a \n so we don't have to use substr.
		line[0] = '\n';
		line.resize(last + 1);
		value += line;
	    }

	    const vector<Action> &v = index_spec[field];
	    string old_value = value;
	    vector<Action>::const_iterator i;
	    bool this_field_is_content = true;
	    for (i = v.begin(); i != v.end(); ++i) {
		switch (i->get_action()) {
		    case Action::BAD:
			abort();
		    case Action::NEW:
			value = old_value;
			// We're processing the same field again - give it a
			// reprieve.
			this_field_is_content = true;
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
			wordcount = index_text(value, doc, stemmer,
					       i->get_num_arg(),
					       i->get_string_arg(), wordcount);
			break;
		    case Action::INDEXNOPOS:
			// No positional information so phrase searching
			// won't work.  However, the database will use much
			// less diskspace.
			index_text(value, doc, stemmer, i->get_num_arg(),
				   i->get_string_arg());
			break;
		    case Action::BOOLEAN:
			doc.add_term(i->get_string_arg() + value);
			break;
		    case Action::HASH: {
			unsigned int max_length = i->get_num_arg();
			if (max_length == 0)
			    max_length = MAX_SAFE_TERM_LENGTH - 1;
			if (value.length() > max_length)
			    value = hash_long_term(value, max_length);
			break;
		    }
		    case Action::LOWER:
			lowercase_term(value);
			break;
		    case Action::LOAD: {
			bool truncated = false;
			if (!load_file(value, i->get_num_arg(), true,
				       value, truncated)) {
			    cerr << "Couldn't load file '" << value << "': "
				 << strerror(errno) << endl;
			    value.resize(0);
			}
			if (!truncated) break;
			/* FALLTHRU (conditionally) */
		    }
		    case Action::TRUNCATE: {
			string::size_type l = i->get_num_arg();
			if (l < value.size()) {
			    // Trim back to (and including) previous whitespace.
			    while (l > 0 && !isspace(static_cast<unsigned char>(value[l - 1]))) --l;
			    while (l > 0 && isspace(static_cast<unsigned char>(value[l - 1]))) --l;

			    // If the first word is too long, just truncate it.
			    if (l == 0) l = i->get_num_arg();

			    value = value.substr(0, l);
			}
			break;
		    }
		    case Action::UNHTML: {
			MyHtmlParser p;
			try {
			    p.parse_html(value);
			} catch (bool) {
			    // MyHtmlParser throws a bool to abandon parsing at
			    // </body> or when indexing is disallowed
			}
			if (p.indexing_allowed)
			    value = p.dump;
			else
			    value = "";
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
		    case Action::DATE: {
			string type = i->get_string_arg();
			switch (type[0]) {
			    case 'u':
				if (type == "unix") {
				    struct tm *tm;
				    time_t t = atoi(value.c_str());
				    tm = localtime(&t);
				    value = date_to_string(tm->tm_year + 1900,
							   tm->tm_mon + 1,
							   tm->tm_mday);
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
		    default:
			/* Empty default case to avoid "unhandled enum value"
			 * warnings. */
			break;
		}
	    }
	    if (this_field_is_content) seen_content = true;
	    if (stream.eof()) break;
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
    }

    // Flush after each file to make sure all changes from that file make it in.
    if (verbose) cout << "Flushing: " << endl;
    database.flush();

    return true;
}

int
main(int argc, char **argv)
{
    // If the database already exists, default to updating not overwriting.
    int database_mode = Xapian::DB_CREATE_OR_OPEN;
    verbose = false;
    Xapian::Stem stemmer("english");

    static const struct option longopts[] = {
	{ "help",	no_argument,	NULL, 'h' },
	{ "version",	no_argument,	NULL, 'V' },
	{ "stemmer",	required_argument,	NULL, 's' },
	{ "overwrite",	no_argument,	NULL, 'o' },
	{ "quiet",	no_argument,	NULL, 'q' },
	{ "verbose",	no_argument,	NULL, 'v' },
	{ 0, 0, NULL, 0 }
    };

    bool more = true, show_help = false;
    while (more) {
	switch (gnu_getopt_long(argc, argv, "uqv", longopts, NULL)) {
	    case EOF:
		more = false;
		break;
	    default:
	    case 'h': // --help
		show_help = true;
		more = false;
		break;
	    case 'V': // --version
		print_package_info(PROG_NAME);
		return 0;
	    case 'o': // --overwrite
		database_mode = Xapian::DB_CREATE_OR_OVERWRITE;
		break;
	    case 'u':
		// Update is now the default, so ignore -u for compatibility.
		break;
	    case 'q':
		// --quiet no longer does anything; ignore for compatibility.
		break;
	    case 'v':
		verbose = true;
		break;
	    case 's':
		try {
		    stemmer = Xapian::Stem(optarg);
		} catch (const Xapian::Error &) {
		    cerr << "Unknown stemming language '" << optarg << "'.\n";
		    cerr << "Available language names are: "
			 << Xapian::Stem::get_available_languages() << endl;
		    return 1;
		}
		break;
	}
    }

    argv += optind;
    argc -= optind;
    if (show_help || argc < 2) {
	cout << PROG_NAME" - "PROG_DESC"\n"
"Usage: "PROG_NAME" [OPTIONS] DATABASE INDEXER_SCRIPT [INPUT_FILE]...\n\n"
"Creates or updates a Xapian database with the data from the input files listed\n"
"on the command line.  If no files are specified, data is read from stdin.\n\n"
"Options:\n"
"  -v, --verbose       display additional messages to aid debugging\n"
"      --overwrite     create the database anew (the default is to update if\n"
"                      the database already exists)\n";
	print_stemmer_help("");
	print_help_and_version_help("", 'V');
	exit(show_help ? 0 : 1);
    }

    parse_index_script(argv[1]);

    // Catch any Xapian::Error exceptions thrown.
    try {
	// Open the database.
	Xapian::WritableDatabase database;
	while (true) {
	    try {
		database = Xapian::WritableDatabase(argv[0], database_mode);
		break;
	    } catch (const Xapian::DatabaseLockError &) {
		// Sleep and retry if we get a Xapian::DatabaseLockError -
		// this just means that another process is updating the
		// database.
		cout << "Database locked ... retrying" << endl;
		sleep(1);
	    }
	}

	addcount = 0;
	repcount = 0;
	delcount = 0;

	if (argc == 2) {
	    // Read from stdin.
	    index_file("<stdin>", cin, database, stemmer);
	} else {
	    // Read file(s) listed on the command line.
	    for (int i = 2; i < argc; ++i) {
		ifstream stream(argv[i]);
		if (stream) {
		    index_file(argv[i], stream, database, stemmer);
		} else {
		    cout << "Can't open file " << argv[i] << endl;
		}
	    }
	}

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
