/* parsequery.yy: parser for omega query strings
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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

%{
#include <config.h>
#include <algorithm>
#include <vector>
#include <string>
#include <list>
#include <map>
#include <om/om.h>
#include <stdio.h>
#include <ctype.h>

using namespace std;
 
class U {
    public:
	OmQuery q;
	vector<OmQuery> v;
	vector<OmQuery> love;
	vector<OmQuery> hate;

	U(OmQuery q) : q(q) { }
	U() { }
};

#define YYSTYPE U

#include "omparsequery.h"

static int yyparse();
static int yyerror(const char *s);
static int yylex();

static OmQuery query;

static OmQueryParser *qp;
static string q;
%}

/* BISON Declarations */
%token TERM PREFIXTERM HYPHEN
%left OR XOR NOT
%left AND
%left NEAR
%left '+' '-'

/* Grammar follows */
%%
input:	  /* nothing */	{ query = OmQuery(); }
	| exp		{ query = $1.q; }
;

exp:	  prob		{
			    OmQuery q = $1.q;
			    if ($1.love.size()) {
				OmQuery love(OmQuery::OP_AND,
					     $1.love.begin(),
					     $1.love.end());
				if (q.is_empty()) {
				    q = love;
				} else {
				    q = OmQuery(OmQuery::OP_AND_MAYBE, love, q);
				}				
			    }
			    if ($1.hate.size()) {
				q = OmQuery(OmQuery::OP_AND_NOT,
					    q,
					    OmQuery(OmQuery::OP_OR,
						    $1.hate.begin(),
						    $1.hate.end()));
			    }
			    $$ = q;
			}
	| exp AND exp	{ $$ = U(OmQuery(OmQuery::OP_AND, $1.q, $3.q)); }
	| exp OR exp	{ $$ = U(OmQuery(OmQuery::OP_OR, $1.q, $3.q)); }
	| exp NOT exp	{ $$ = U(OmQuery(OmQuery::OP_AND_NOT, $1.q, $3.q)); }
	| exp XOR exp	{ $$ = U(OmQuery(OmQuery::OP_XOR, $1.q, $3.q)); }
	| '(' exp ')'	{ $$ = $2; }
	/* error catches */
	| exp AND	{ throw "Syntax: expression AND expression"; }
	| AND exp	{ throw "Syntax: expression AND expression"; }
	| exp OR	{ throw "Syntax: expression OR expression"; }
	| OR exp	{ throw "Syntax: expression OR expression"; }
	| exp NOT	{ throw "Syntax: expression NOT expression"; }
	| NOT exp	{ throw "Syntax: expression NOT expression"; }
	| exp XOR	{ throw "Syntax: expression XOR expression"; }
	| XOR exp	{ throw "Syntax: expression XOR expression"; }
;

prob:	  probterm
	| prob probterm	{ if ($1.q.is_empty()) {
	    		      $$ = $2; // even if $2.q.is_empty()
			  } else if ($2.q.is_empty()) {
			      $$ = $1;
			  } else {
			      $$ = U(OmQuery(qp->default_op, $1.q, $2.q));
			  }
	                  $$.love = $1.love;
	                  $$.hate = $1.hate; }			  
	| '+' term	{ $$.love.push_back($2.q); }
	| prob '+' term	{ $$ = $1; $$.love.push_back($3.q); }
	| '-' term	{ $$.hate.push_back($2.q); }
	| prob '-' term	{ $$ = $1; $$.hate.push_back($3.q); }
;

probterm: stopterm
	| PREFIXTERM
;

stopterm: TERM		{ string term = *($1.q.get_terms_begin()); 
			  if (qp->stop && (*qp->stop)(term)) {
			      $$ = U();
			      qp->stoplist.push_back(term);
			      // This is ugly - FIXME?
			      list<string>::iterator i, j;
			      i = qp->termlist.begin();
			      do {
				  j = i;
				  ++i;
				  i = find(i, qp->termlist.end(), term);
			      } while (i != qp->termlist.end());
			      qp->termlist.erase(j);
			  } else {
			      $$ = $1;
			  } }
	| '"' phrase '"'{ $$ = U(OmQuery(OmQuery::OP_PHRASE, $2.v.begin(), $2.v.end()));
			  $$.q.set_window($2.v.size()); }
	| hypphr        { $$ = U(OmQuery(OmQuery::OP_PHRASE, $1.v.begin(), $1.v.end()));
			  $$.q.set_window($1.v.size()); }
	| nearphr	{ $$ = U(OmQuery(OmQuery::OP_NEAR, $1.v.begin(), $1.v.end()));
			  $$.q.set_window($1.v.size() + 9); }
;

term:	  TERM
	| PREFIXTERM
	| '"' phrase '"'{ $$ = U(OmQuery(OmQuery::OP_PHRASE, $2.v.begin(), $2.v.end()));
			  $$.q.set_window($2.v.size()); }
	| hypphr        { $$ = U(OmQuery(OmQuery::OP_PHRASE, $1.v.begin(), $1.v.end()));
			  $$.q.set_window($1.v.size()); }
	| nearphr	{ $$ = U(OmQuery(OmQuery::OP_NEAR, $1.v.begin(), $1.v.end()));
			  $$.q.set_window($1.v.size() + 9); }
;

phrase:	  TERM		{ $$.v.push_back($1.q); }
	| phrase TERM	{ $$ = $1; $$.v.push_back($2.q); }
;

hypphr:   TERM HYPHEN TERM	{ $$.v.push_back($1.q); $$.v.push_back($3.q); }
	| hypphr HYPHEN TERM	{ $$ = $1; $$.v.push_back($3.q); }
;

nearphr:  TERM NEAR TERM	{ $$.v.push_back($1.q); $$.v.push_back($3.q); }
	| nearphr NEAR TERM	{ $$ = $1; $$.v.push_back($3.q); }
;

%%

static string::iterator qptr;
static int pending_token;
static om_termpos termpos;
static string prefix;

void
OmQueryParser::set_stemming_options(const string &lang, bool stem_all_,
				  OmStopper * stop_)
{
    if (stop) delete stop;
    stop = stop_;
    if (lang.empty()) {
	stem = false;
    } else {
	if (stemmer) delete stemmer;
	stemmer = new OmStem(lang);
	stem = true;
	stem_all = stem_all_;
    }
}

/* Lexical analyzer.  Puts a U object on the stack and returns a token.
 * The token can be TERM, a token for an operator, the ASCII character
 * read, or EOF.  Skips all whitespace (but whitespace is used to resolve
 * operators with multiple meanings: e.g. `-' can be a hyphen or exclude a
 * term).
 */

static inline int
next_char()
{
   if (qptr == q.end()) return EOF;
   return *qptr++;
}

// FIXME: copied from om/indexer/index_utils.cc
static void
lowercase_term(string &term)
{
    string::iterator i = term.begin();
    while(i != term.end()) {
	*i = tolower(*i);
	i++;
    }
}

inline static bool
p_notalnum(char c)
{
    return !isalnum(c);
}

inline static bool
p_whitespace(char c)
{
    return isspace(c);
}

inline static bool
p_notwhitespace(char c)
{
    return !isspace(c);
}

inline static bool
p_notplusminus(unsigned int c)
{
    return c != '+' && c != '-';
}

int
yylex()
{
    int c;
    if (pending_token) {
	c = pending_token;
	pending_token = 0;
	return c;
    }
    
    if (prefix.empty()) {
	/* skip whitespace */
	qptr = find_if(qptr, q.end(), p_notwhitespace);
	if (qptr == q.end()) return 0;
    }

    /* process terms */
    if (isalnum(*qptr)) {
	string term, original_term;
	bool already_stemmed = !qp->stem;
	string::iterator term_start = qptr, term_end;
	// This needs more work - e.g. keywords have no positional info so this
	// must not fire for a phrase search
#ifdef AUTO_SEARCH_KEYWORDS 
	// See if we've got a term with punctuation in by checking if
	// the longest non-whitespace string is a keyword.
	// So if there's a term "Kb*witched", you can search for
	// `B*witched concert tickets'.
	term_end = find_if(term_start, q.end(), p_whitespace);
	if (find_if(term_start, term_end, p_notalnum) != term_end) {
	    // It contains punctuation, so see if it's a keyword
	    term = q.substr(term_start - q.begin(), term_end - term_start);
	    original_term = term;
	    lowercase_term(term);
	    term = "K" + term;
	    if (qp->db.term_exists("K" + term)) {
		// It's a keyword, so don't stem it
		already_stemmed = true;
	    } else {
		original_term = "";
		term = "";
	    }
	}
#endif
	if (term.empty() && isupper(*qptr)) {
	    term = *qptr;
	    while (++qptr != q.end() && *qptr == '.' &&
		   ++qptr != q.end() && isupper(*qptr)) {
		term += *qptr;
	    }
	    if (term.length() < 2 || (qptr != q.end() && isalnum(*qptr))) {
		qptr = term_start;
		term = "";
	    }
	}
	if (term.empty()) {
more_term:
	    term_end = find_if(qptr, q.end(), p_notalnum);
	    if (term_end != q.end()) {
		if (*term_end == '&') {
		    // Treat AT&T M&S A&P etc as a single term
		    if (term_end + 1 != q.end() && isalnum(term_end[1])) {
			qptr = term_end + 1;
			goto more_term;
		    }
		} else {
		    string::iterator end2;
		    end2 = find_if(term_end, q.end(), p_notplusminus);
		    if (end2 == q.end() || !isalnum(*end2)) term_end = end2;
		}
	    }
	    term = q.substr(term_start - q.begin(), term_end - term_start);
	    qptr = term_end;
	}

	if (qptr != q.end()) {
	    if (*qptr == '.') {
		// "example.com" should give "exampl" and "com" - need EOF or
		// space after '.' to mean "don't stem"
		qptr++;
		if (qptr == q.end() || isspace(*qptr)) {
		    if (original_term.empty()) original_term = term + '.';
		    already_stemmed = true;
		}
	    }
	    if (*qptr == '-') {
		if (qptr + 1 != q.end() && isalnum(*(qptr + 1))) {
		    qptr++;
		    pending_token = HYPHEN;
		}
	    }
	    if (prefix.empty() && *qptr == ':') {
		string::iterator saved_qptr = qptr;
		++qptr;
		if (qptr != q.end() && !isspace(*qptr)) {
		    map<string, string>::const_iterator f;
		    f = qp->prefixes.find(term);
		    if (f != qp->prefixes.end()) {
		       prefix = f->second;
		       // FIXME: what about boolean prefix terms?
		       // search for "subject:xapian anon cvs" should
		       // use default_op (or should it?) but search
		       // for "cvs site:xapian.org" should use FILTER
		       //
		       // FIXME: Also, what about prefix terms in NEAR,
		       // prefixed phrases (subject:"space flight")
		       if (yylex() == TERM) return PREFIXTERM;
		       prefix = "";
		    }
		}
		qptr = saved_qptr;
	    }
	}

	// Boolean operators
	if (prefix.empty()) {
	    if (term == "AND") {
		return AND;
	    } else if (term == "OR") {
		return OR;
	    } else if (term == "NOT") {
		return NOT;
	    } else if (term == "XOR") {
		return XOR;
	    } else if (term == "NEAR") {
		return NEAR;
	    }
	}

	bool raw_term = (!already_stemmed && !qp->stem_all && !islower(term[0]));
	if (original_term.empty()) original_term = term;
	lowercase_term(term);
	if (raw_term)
	    term = 'R' + term;
	else if (!already_stemmed)
	    term = qp->stemmer->stem_word(term);
	if (!prefix.empty()) {
	    if (prefix.length() > 1 && (isupper(term[0]) || isdigit(term[0]))) {
		prefix += ':';
	    }
	    term = prefix + term;
	    prefix = "";
	}
	yylval = U(OmQuery(term, 1, termpos++));
	qp->termlist.push_back(term);
	qp->unstem.insert(make_pair(term, original_term));
	return TERM;
    }
    c = *qptr++;
    switch (c) {
#if 0 // Most people won't want these, but make configurable somehow?
     case '&':
	return AND;
     case '|':
	return OR;
#endif
     // FIXME: Make this list configurable
     case '_': case '/': case '\\': case '@':
     case '\'': case '*':
	/* these characters generate a phrase search */
	if (isalnum(*qptr)) return HYPHEN;
	break;
     case '(': case ')': case '-': case '+': case '"':
	/* these characters are used in the grammar rules */
	return c;
    }
    /* ignore any other characters */
    return yylex();                                
}

int
yyerror(const char *s)
{
    throw s;
}

OmQuery
OmQueryParser::parse_query(const string &q_)
{
    qp = this;
    q = q_;
    pending_token = 0;
    termpos = 1;
    prefix = "";
    qptr = q.begin();
    if (yyparse() == 1) {
	throw "query failed to parse";
    }
    OmQuery res = query;
    query = OmQuery();
    q = "";
    return res;
}

#if 0
// FIXME
// transliterate accented characters in step with what the indexers do
static int
get_next_char(const char **p)
{
    static int cache = 0;
    int ch;
    if (cache) {
	ch = cache;
	cache = 0;
	return ch;
    }
    ch = (int)(unsigned char)(*(*p)++);
    switch (ch) {
#include "symboltab.h"
    }
    return ch;
}
#endif

/* Tell vim this is a yacc file
 * vim: syntax=yacc
 */
