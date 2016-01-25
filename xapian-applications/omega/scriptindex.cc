/* scriptindex.cc
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Sam Liddicott
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2014,2015 Olly Betts
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
#include <cstring>

#include <cstdlib>
#include "safeerrno.h"
#include <cstdio>
#include <ctime>
#include "safeunistd.h"

#include "commonhelp.h"
#include "hashterm.h"
#include "loadfile.h"
#include "myhtmlparse.h"
#include "stringutils.h"
#include "utf8truncate.h"
#include "utils.h"

#include "gnu_getopt.h"

using namespace std;

#define PROG_NAME "scriptindex"
#define PROG_DESC "index arbitrary data as described by an index script"

static bool verbose;
static int addcount;
static int repcount;
static int delcount;

inline static bool
p_space(unsigned int c)
{
    return C_isspace(c);
}

inline static bool
p_notspace(unsigned int c)
{
    return !C_isspace(c);
}

inline static bool
p_notalpha(unsigned int c)
{
    return !C_isalpha(c);
}

// Characters allowed as second or subsequent characters in a fieldname
inline static bool
p_notfieldnamechar(unsigned int c)
{
    return !C_isalnum(c) && c != '_';
}

inline bool
prefix_needs_colon(const string & prefix, unsigned ch)
{
    if (!C_isupper(ch)) return false;
    string::size_type len = prefix.length();
    return (len > 1 && prefix[len - 1] != ':');
}

const char * action_names[] = {
    "bad", "new",
    "boolean", "date", "field", "hash", "index", "indexnopos", "load", "lower",
    "spell", "truncate", "unhtml", "unique", "value", "valuenumeric", "weight"
};

// For debugging:
#define DUMP_ACTION(A) cout << action_names[(A).get_action()] << "(" << (A).get_string_arg() << "," << (A).get_num_arg() << ")" << endl

class Action {
public:
    typedef enum {
	BAD, NEW,
	BOOLEAN, DATE, FIELD, HASH, INDEX, INDEXNOPOS, LOAD, LOWER,
	SPELL, TRUNCATE, UNHTML, UNIQUE, VALUE, VALUENUMERIC, WEIGHT
    } type;
private:
    type action;
    int num_arg;
    string string_arg;
public:
    Action(type action_) : action(action_), num_arg(0) { }
    Action(type action_, const string & arg)
	: action(action_), string_arg(arg) {
	num_arg = atoi(string_arg.c_str());
    }
    Action(type action_, const string & arg, int num)
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
	    if (!C_isalnum(*i)) {
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
		    case 's':
			if (action == "spell") {
			    code = Action::SPELL;
			}
			break;
		    case 't':
			if (action == "truncate") {
			    code = Action::TRUNCATE;
			    arg = YES;
			    takes_integer_argument = true;
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
			} else if (action == "valuenumeric") {
			    code = Action::VALUENUMERIC;
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
		string val(j, i);
		if (takes_integer_argument) {
		    if (val.find('.') != string::npos) {
			cout << filename << ':' << line_no
			     << ": Warning: Index action '" << action
			     << "' takes an integer argument" << endl;
		    }
		}
		switch (code) {
		    case Action::INDEX:
			if (val == "nopos") {
			    // INDEX used to take an optional argument which
			    // could be "nopos" to mean the same that
			    // INDEXNOPOS now does.  FIXME:1.3.0 remove this
			    // error eventually
			    cerr << filename << ':' << line_no
				 << ": Support for 'index=nopos' has been "
				    "removed - use 'indexnopos' instead"
				 << endl;
			    exit(1);
			}
			/* FALLTHRU */
		    case Action::INDEXNOPOS:
			actions.push_back(Action(code, val, weight));
			useless_weight_pos = string::npos;
			break;
		    case Action::WEIGHT:
			// We don't push an Action for WEIGHT - instead we
			// store it ready to use in the INDEX and INDEXNOPOS
			// Actions.
			weight = atoi(val.c_str());
			if (useless_weight_pos != string::npos) {
			    report_useless_action(filename, line_no,
						  useless_weight_pos, action);
			}
			useless_weight_pos = j - s.begin();
			break;
		    case Action::TRUNCATE:
			if (!actions.empty() &&
			    actions.back().get_action() == Action::LOAD) {
			    /* Turn "load truncate=n" into "load" with
			     * num_arg n, so that we don't needlessly
			     * allocate memory and read data we're just
			     * going to ignore.
			     */
			    actions.pop_back();
			    code = Action::LOAD;
			}
			actions.push_back(Action(code, val));
			break;
		    case Action::UNIQUE:
			if (boolmap.find(val) == boolmap.end())
			    boolmap[val] = Action::UNIQUE;
			actions.push_back(Action(code, val));
			break;
		    case Action::BOOLEAN:
			boolmap[val] = Action::BOOLEAN;
			/* FALLTHRU */
		    default:
			actions.push_back(Action(code, val));
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
		case Action::SPELL:
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

    if (index_spec.empty()) {
	cout << filename << ": No rules found in index script" << endl;
	exit(1);
    }
}

static bool
index_file(const char *fname, istream &stream,
	   Xapian::WritableDatabase &database, Xapian::TermGenerator &indexer)
{
    string line;
    size_t line_no = 0;
    while (!stream.eof() && getline(stream, line)) {
	++line_no;
	Xapian::Document doc;
	indexer.set_document(doc);
	Xapian::docid docid = 0;
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
	    while (getline(stream, line)) {
		++line_no;
		if (line.empty() || line[0] != '=') break;
		// Cope with files from MS Windows (\r\n end of lines).
		// Trim multiple \r characters, since that seems the best way
		// to handle that case.
		last = line.find_last_not_of('\r');
		// line[0] == '=', so last != string::npos.
		// Replace the '=' with a '\n' so we don't have to use substr.
		line[0] = '\n';
		line.resize(last + 1);
		value += line;
	    }

	    // Default to not indexing spellings.
	    indexer.set_flags(Xapian::TermGenerator::flags(0));

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
			    string::size_type j = 0;
			    while ((j = s.find('\n', j)) != string::npos)
				s[j] = ' ';
			    fields[f].push_back(s);
			}
			break;
		    case Action::INDEX:
			indexer.index_text(value,
					   i->get_num_arg(),
					   i->get_string_arg());
			break;
		    case Action::INDEXNOPOS:
			// No positional information so phrase searching
			// won't work.  However, the database will use much
			// less diskspace.
			indexer.index_text_without_positions(value,
							     i->get_num_arg(),
							     i->get_string_arg());
			break;
		    case Action::BOOLEAN: {
			// Do nothing if there's no text.
			if (value.empty()) break;

			string term = i->get_string_arg();
			if (prefix_needs_colon(term, value[0])) term += ':';
			term += value;

			doc.add_boolean_term(term);
			break;
		    }
		    case Action::HASH: {
			unsigned int max_length = i->get_num_arg();
			if (max_length == 0)
			    max_length = MAX_SAFE_TERM_LENGTH - 1;
			if (value.length() > max_length)
			    value = hash_long_term(value, max_length);
			break;
		    }
		    case Action::LOWER:
			value = Xapian::Unicode::tolower(value);
			break;
		    case Action::LOAD: {
			bool truncated = false;
			// FIXME: Use NOATIME if we own the file or are root.
			if (!load_file(value, i->get_num_arg(), NOCACHE,
				       value, truncated)) {
			    cerr << "Couldn't load file '" << value << "': "
				 << strerror(errno) << endl;
			    value.resize(0);
			}
			if (!truncated) break;
			/* FALLTHRU (conditionally) */
		    }
		    case Action::TRUNCATE:
			utf8_truncate(value, i->get_num_arg());
			break;
		    case Action::SPELL:
			indexer.set_flags(indexer.FLAG_SPELLING);
			break;
		    case Action::UNHTML: {
			MyHtmlParser p;
			try {
			    // Default HTML character set is latin 1, though
			    // not specifying one is deprecated these days.
			    p.parse_html(value, "iso-8859-1", false);
			} catch (const string & newcharset) {
			    p.reset();
			    p.parse_html(value, newcharset, true);
			}
			if (p.indexing_allowed)
			    value = p.dump;
			else
			    value = "";
			break;
		    }
		    case Action::UNIQUE: {
			// If there's no text, just issue a warning.
			if (value.empty()) {
			    cout << fname << ':' << line_no
				 << ": Ignoring UNIQUE action on empty text"
				 << endl;
			    break;
			}

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
			if (prefix_needs_colon(t, value[0])) t += ':';
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
			    cout << "E: " << e.get_description() << endl;
			    database.commit();
			    goto again;
			}
			break;
		    }
		    case Action::VALUE:
			if (!value.empty())
			    doc.add_value(i->get_num_arg(), value);
			break;
		    case Action::VALUENUMERIC: {
			if (value.empty()) break;
			char * end;
			double dbl = strtod(value.c_str(), &end);
			if (*end) {
			    cout << fname << ':' << line_no << ": Warning: "
				    "Trailing characters in VALUENUMERIC: '"
				 << value << "'" << endl;
			}
			doc.add_value(i->get_num_arg(),
				      Xapian::sortable_serialise(dbl));
			break;
		    }
		    case Action::DATE: {
			const string & type = i->get_string_arg();
			string yyyymmdd;
			if (type == "unix") {
			    time_t t = atoi(value.c_str());
			    struct tm *tm = localtime(&t);
			    int y = tm->tm_year + 1900;
			    int m = tm->tm_mon + 1;
			    yyyymmdd = date_to_string(y, m, tm->tm_mday);
			} else if (type == "yyyymmdd") {
			    if (value.length() == 8) yyyymmdd = value;
			}
			if (yyyymmdd.empty()) break;
			// Date (YYYYMMDD)
			doc.add_boolean_term("D" + yyyymmdd);
			yyyymmdd.resize(6);
			// Month (YYYYMM)
			doc.add_boolean_term("M" + yyyymmdd);
			yyyymmdd.resize(4);
			// Year (YYYY)
			doc.add_boolean_term("Y" + yyyymmdd);
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
		    cout << "E: " << e.get_description() << endl;
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

    // Commit after each file to make sure all changes from that file make it
    // in.
    if (verbose) cout << "Committing: " << endl;
    database.commit();

    return true;
}

int
main(int argc, char **argv)
try {
    // If the database already exists, default to updating not overwriting.
    int database_mode = Xapian::DB_CREATE_OR_OPEN;
    verbose = false;
    Xapian::Stem stemmer("english");

    static const struct option longopts[] = {
	{ "help",	no_argument,	NULL, 'h' },
	{ "version",	no_argument,	NULL, 'V' },
	{ "stemmer",	required_argument,	NULL, 's' },
	{ "overwrite",	no_argument,	NULL, 'o' },
	{ "verbose",	no_argument,	NULL, 'v' },
	{ 0, 0, NULL, 0 }
    };

    bool more = true, show_help = false;
    while (more) {
	switch (gnu_getopt_long(argc, argv, "vs:hV", longopts, NULL)) {
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
	    case 'v':
		verbose = true;
		break;
	    case 's':
		try {
		    stemmer = Xapian::Stem(optarg);
		} catch (const Xapian::InvalidArgumentError &) {
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
	cout << PROG_NAME " - " PROG_DESC "\n"
"Usage: " PROG_NAME " [OPTIONS] DATABASE INDEXER_SCRIPT [INPUT_FILE]...\n"
"\n"
"Creates or updates a Xapian database with the data from the input files listed\n"
"on the command line.  If no files are specified, data is read from stdin.\n"
"\n"
"See https://xapian.org/docs/omega/scriptindex.html for documentation of the\n"
"format for INDEXER_SCRIPT.\n"
"\n"
"Options:\n"
"  -v, --verbose       display additional messages to aid debugging\n"
"      --overwrite     create the database anew (the default is to update if\n"
"                      the database already exists)\n";
	print_stemmer_help("");
	print_help_and_version_help("");
	exit(show_help ? 0 : 1);
    }

    parse_index_script(argv[1]);

    // Open the database.
    Xapian::WritableDatabase database;
    while (true) {
	try {
	    database = Xapian::WritableDatabase(argv[0], database_mode);
	    break;
	} catch (const Xapian::DatabaseLockError &) {
	    // Sleep and retry if we get a Xapian::DatabaseLockError - this
	    // just means that another process is updating the database.
	    cout << "Database locked ... retrying" << endl;
	    sleep(1);
	}
    }

    Xapian::TermGenerator indexer;
    indexer.set_stemmer(stemmer);
    // Set the database for spellings to be added to by the "spell" action.
    indexer.set_database(database);

    addcount = 0;
    repcount = 0;
    delcount = 0;

    if (argc == 2) {
	// Read from stdin.
	index_file("<stdin>", cin, database, indexer);
    } else {
	// Read file(s) listed on the command line.
	for (int i = 2; i < argc; ++i) {
	    ifstream stream(argv[i]);
	    if (stream) {
		index_file(argv[i], stream, database, indexer);
	    } else {
		cout << "Can't open file " << argv[i] << endl;
	    }
	}
    }

    cout << "records (added, replaced, deleted) = (" << addcount << ", "
	 << repcount << ", " << delcount << ")" << endl;
} catch (const Xapian::Error &error) {
    cout << "Exception: " << error.get_description() << endl;
    exit(1);
} catch (const std::bad_alloc &) {
    cout << "Exception: std::bad_alloc" << endl;
    exit(1);
} catch (...) {
    cout << "Unknown Exception" << endl;
    exit(1);
}
