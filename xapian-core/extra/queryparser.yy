/* queryparser.yy: parser for omega-like query strings
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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

%{
#include <config.h>
#include <algorithm>
#include <vector>
#include <string>
#include <list>
#include <map>
#include <xapian.h>
#include <stdio.h>
#include <ctype.h>

using namespace std;
using namespace Xapian;
 
class U {
    public:
	Query q;
	vector<Query> v;
	vector<Query> love;
	vector<Query> hate;

	U(Query q) : q(q) { }
	U() { }
};

#define YYSTYPE U

#include <xapian/queryparser.h>

static vector<string> group_prefix;

static int yyparse();
static int yyerror(const char *s);
static int yylex();

static Query query;

static QueryParser *qp;
static string q;
%}

/* BISON Declarations */
%token TERM PREFIXTERM HYPHEN PREFIXQUOTE PREFIXBRA
%left OR XOR NOT
%left AND
%left NEAR
%left '+' '-'

/* Grammar follows */
%%
input:	  /* nothing */	{ query = Query(); }
	| exp		{ query = $1.q; }
;

boolarg:  exp		{ $$ = $1; }
	| /* nothing */ { $$.q = Query(); }
;

exp:	  prob
	    {
		Query q = $1.q;
		if ($1.love.size()) {
		    Query love(Query::OP_AND, $1.love.begin(), $1.love.end());
		    if (q.is_empty()) {
			q = love;
		    } else {
			q = Query(Query::OP_AND_MAYBE, love, q);
		    }				
		}
		if ($1.hate.size()) {
		    q = Query(Query::OP_AND_NOT, q,
			      Query(Query::OP_OR,
				    $1.hate.begin(), $1.hate.end()));
		}
		$$ = q;
	    }
	| boolarg AND boolarg
	    {   if ($1.q.is_empty() || $3.q.is_empty())
		    throw "Syntax: <expression> AND <expression>";
		$$ = U(Query(Query::OP_AND, $1.q, $3.q)); }
	| boolarg OR boolarg
	    {	if ($1.q.is_empty() || $3.q.is_empty())
		    throw "Syntax: <expression> OR <expression>";
		$$ = U(Query(Query::OP_OR, $1.q, $3.q)); }
	| boolarg NOT boolarg
	    {	if ($1.q.is_empty() || $3.q.is_empty())
		    throw "Syntax: <expression> NOT <expression>";
		$$ = U(Query(Query::OP_AND_NOT, $1.q, $3.q)); }
	| boolarg XOR boolarg
	    {	if ($1.q.is_empty() || $3.q.is_empty())
		    throw "Syntax: <expression> XOR <expression>";
		$$ = U(Query(Query::OP_XOR, $1.q, $3.q)); }
	| '(' exp ')'	{ $$ = $2; }
	| PREFIXBRA	{ group_prefix.push_back(*($1.q.get_terms_begin())); }
	  exp ')'	{ group_prefix.pop_back();
			  $$ = $3; }
;

prob:	  probterm
	| prob probterm	{ if ($1.q.is_empty()) {
	    		      $$ = $2; // even if $2.q.is_empty()
			  } else if ($2.q.is_empty()) {
			      $$ = $1;
			  } else {
			      $$ = U(Query(qp->default_op, $1.q, $2.q));
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

ignorehypterm:	  TERM
		| HYPHEN TERM   { $$ = $2; } /* Ignore leading HYPHEN */
		| '(' ignorehypterm ')'	{ $$ = $2; }
;

stopterm: ignorehypterm	{ string term = *($1.q.get_terms_begin()); 
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
	| PREFIXQUOTE	{ group_prefix.push_back(*($1.q.get_terms_begin())); }
	  phrase '"'	{ group_prefix.pop_back();
			  $$ = U(Query(Query::OP_PHRASE, $3.v.begin(), $3.v.end()));
			  $$.q.set_window($3.v.size()); }
	| '"' phrase '"'{ $$ = U(Query(Query::OP_PHRASE, $2.v.begin(), $2.v.end()));
			  $$.q.set_window($2.v.size()); }
	| hypphr        { $$ = U(Query(Query::OP_PHRASE, $1.v.begin(), $1.v.end()));
			  $$.q.set_window($1.v.size()); }
	| nearphr	{ $$ = U(Query(Query::OP_NEAR, $1.v.begin(), $1.v.end()));
			  $$.q.set_window($1.v.size() + 9); }
;

term:	  ignorehypterm	{ $$ = $1; }
	| PREFIXTERM
	| '"' phrase '"'{ $$ = U(Query(Query::OP_PHRASE, $2.v.begin(), $2.v.end()));
			  $$.q.set_window($2.v.size()); }
	| hypphr        { $$ = U(Query(Query::OP_PHRASE, $1.v.begin(), $1.v.end()));
			  $$.q.set_window($1.v.size()); }
	| nearphr	{ $$ = U(Query(Query::OP_NEAR, $1.v.begin(), $1.v.end()));
			  $$.q.set_window($1.v.size() + 9); }
;

phrase:	  ignorehypterm		{ $$.v.push_back($1.q); }
	| phrase ignorehypterm	{ $$ = $1; $$.v.push_back($2.q); }
;

hypphr:   TERM HYPHEN TERM	{ $$.v.push_back($1.q); $$.v.push_back($3.q); }
	| HYPHEN TERM HYPHEN TERM /* Ignore leading HYPHEN */
				{ $$.v.push_back($2.q); $$.v.push_back($4.q); }
	| hypphr HYPHEN TERM	{ $$ = $1; $$.v.push_back($3.q); }
;

nearphr:  TERM NEAR TERM	{ $$.v.push_back($1.q); $$.v.push_back($3.q); }
	| nearphr NEAR TERM	{ $$ = $1; $$.v.push_back($3.q); }
;

%%

static string::iterator qptr;
static int pending_token;
static termpos term_pos;
static string prefix;

void
QueryParser::set_stemming_options(const string &lang, bool stem_all_,
					  Stopper * stop_)
{
    if (stop) delete stop;
    stop = stop_;
    if (lang.empty() || lang == "none") {
	stem = false;
    } else {
	if (stemmer) delete stemmer;
	stemmer = new Stem(lang);
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
    while (i != term.end()) {
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

static int
yylex()
#ifdef DEBUG_YYLEX
{
    static int yylex2();
    int r = yylex2();
    printf("(%c) %d @%d\n", isprint(r) ? r : '?', r, qptr - q.begin() - 1);
    return r;
}

static int
yylex2()
#endif
{
    int c;
    // Whitespace nullifies pending hyphenation.
    if (pending_token == HYPHEN) {
	while (qptr != q.end() && strchr("_/\\@'*.", *qptr)) ++qptr;
	if (qptr == q.end() || isspace(*qptr)) {
	    pending_token = 0;
	}
    }

    if (pending_token) {
	c = pending_token;
	pending_token = 0;
	return c;
    }
    
    if (qptr == q.end()) return 0;

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
		    if (isupper(term[0])) {
			// Can't have come from relevance feedback - must be
			// something like E.T. or pasted text...
		    } else {
			if (original_term.empty()) original_term = term + '.';
			already_stemmed = true;
		    }
		} else {
		    pending_token = HYPHEN;
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
		       if (*qptr == '"' || *qptr == '(') {
			   // prefixed phrases: subject:"space flight"
			   // prefixed experessions: extract:(fast NEAR food)
			   yylval = U(Query(prefix, 1, 1));
			   prefix = "";
			   return (*qptr++ == '"' ? PREFIXQUOTE : PREFIXBRA);
		       }
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

	bool raw_term = (!already_stemmed && !qp->stem_all && isupper(term[0]));
	if (original_term.empty()) original_term = term;
	lowercase_term(term);
	if (raw_term)
	    term = 'R' + term;
	else if (!already_stemmed)
	    term = qp->stemmer->stem_word(term);
	if (prefix.empty() && !group_prefix.empty())
	    prefix = group_prefix.back();
	if (!prefix.empty()) {
	    if (prefix.length() > 1 && *(prefix.back) != ':') {
		unsigned char ch = (unsigned char)term[0];
		if (!isupper(*(prefix.back)) || isupper(ch) || isdigit(ch))) {
		    prefix += ':';
		}
	    }
	    term = prefix + term;
	    prefix = "";
	}
	yylval = U(Query(term, 1, term_pos++));
	qp->termlist.push_back(term);
#ifndef __SUNPRO_CC
	qp->unstem.insert(make_pair(term, original_term));
#else
	qp->unstem.insert(make_pair<const string, string>(term, original_term));
#endif
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
     // FIXME: Make this list configurable (also '=' ?)
     case '_': case '/': case '\\': case '@':
     case '\'': case '*':
	/* these characters generate a phrase search */
	if (qptr - 1 == q.begin()) {
	    /* Ignore at start of query */
	    return yylex();
	}
	// Ignore at end of query
	if (qptr == q.end()) return 0;

	// Ignore if not preceded by alphanumerics
	if (!isalnum(*(qptr - 2))) break;

	while (!isalnum(*qptr)) {
	    // Skip multiple phrase generaters
	    if (!strchr("_/\\@'*.", *qptr)) return yylex();
	    ++qptr;
	    // Ignore at end of query
	    if (qptr == q.end()) return 0;
	}
	return HYPHEN;
     case '(':
	// Ignore ( at end of query
	if (qptr == q.end()) return 0;
	if (*qptr == ')') {
	    /* Ignore () */
	    ++qptr;
	    return yylex();
	}
	/* '(' is used in the grammar rules */
	return c;
     case '+':
	// Ignore + at end of query
	if (qptr == q.end()) return 0;
	if (isspace(*qptr) || *qptr == '+') {
	    /* Ignore ++ or + followed by a space */
	    /* Note that C++ and Mg2+ are handled above */
	    ++qptr;
	    return yylex();
	}
	/* '+' is used in the grammar rules */
	return c;
     case '-':
	// Ignore - at end of query
	if (qptr == q.end()) return 0;
	if (isspace(*qptr) || *qptr == '-') {
	    /* Ignore -- or - followed by a space */
	    /* Note that nethack-- and Cl- are handled above */
	    ++qptr;
	    return yylex();
	}
	/* '-' is used in the grammar rules */
	return c;
     case '"':
	if (qptr != q.end() && *qptr == '"') {
	    /* Ignore "" */
	    ++qptr;
	    return yylex();
	}
	/* '"' is used in the grammar rules */
	return c;
     case ')':
	if (qptr - 1 == q.begin()) {
	    /* Ignore ) at start of query */
	    return yylex();
	}
	/* ')' is used in the grammar rules */
	return c;
    }
    /* ignore any other characters */
    return yylex();                                
}

static int
yyerror(const char *s)
{
    s = s; // Not used
    return 0;
}

Query
QueryParser::parse_query(const string &q_)
{
    qp = this;

    q.erase();
    q.reserve(q_.size());

    string::const_iterator i = q_.begin();
    // Skip leading whitespace
    while (isspace((unsigned char)*i)) ++i;
    
    for ( ; i != q_.end(); ++i) {
	int ch = (unsigned char)*i;
	if (ch < 32 || ch == 127) ch = ' ';
	int cache = 0;
	// Transliterate accented characters in step with what the indexers do
	// FIXME: This means that N&Oacute;T in a query is converted to NOT
	// which is taken as a boolean operator - we probably don't want to
	// do this here.  However, we probably do want to transliterate &mdash;
	// to a "-", etc.
	switch (ch) {
#include "symboltab.h"
	}
	q += (char)ch;
	if (cache) q += (char)cache;
    }

    if (q.empty()) return Query();

    // Skip trailing whitespace
    i = q.end();
    while (isspace((unsigned char)i[-1])) --i;
    q.erase(i - q.begin());
 
#if 0 
    printf("[%s]\n", q.c_str());
#endif
    
    pending_token = 0;
    term_pos = 1;
    prefix = "";
    qptr = q.begin();

    if (yyparse() == 1) {
	// Strip out any non-alphanumerics and try to parse again.
	// FIXME: be smarter about certain non-alphanumerics...
	string::iterator j = q.begin();
	while (true) {
	    j = find_if(j, q.end(), p_notalnum);
	    if (j == q.end()) break;
	    if (*j != '.') *j = ' ';
	    ++j;
	}

	pending_token = 0;
	term_pos = 1;
	prefix = "";
	qptr = q.begin();

	if (yyparse() == 1) {
	    throw "parse error";
	}
    }

    Query res = query;
    query = Query();
    q.erase();
    return res;
}

/* Tell vim this is a yacc file
 * vim: syntax=yacc
 */
