/* parsequery.yy: parser for omega query strings
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Ananova Ltd
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
#include <vector>
#include <string>
#include <om/om.h>
#include <stdio.h>
#include <ctype.h>
using std::vector;
using std::string;

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

#include "parsequery.h"

static int yyparse();
static int yyerror(const char *s);
static int yylex();

static OmQuery query;

static QueryParser *qp;
static string q;
%}

/* BISON Declarations */
%token TERM HYPHEN
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
;

prob:	  stopterm
	| prob stopterm	{ if ($1.q.is_empty()) {
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

stopterm: TERM		{ om_termname term = *($1.q.get_terms_begin()); 
			  if (qp->stop && (*qp->stop)(term)) {
			      $$ = U();
			      qp->stoplist.push_back(term);
			      // This is ugly - FIXME?
			      list<om_termname>::iterator i, j;
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
static bool stem, stem_all;
OmStem *stemmer;

void
QueryParser::set_stemming_options(const string &lang, bool stem_all_,
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
lowercase_term(om_termname &term)
{
    om_termname::iterator i = term.begin();
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
    
    /* skip whitespace */
    qptr = find_if(qptr, q.end(), p_notwhitespace);
    if (qptr == q.end()) return 0;

    /* process terms */
    if (isalnum(*qptr)) {
	string term;
	bool already_stemmed = !stem;
	string::iterator term_end;
	term_end = find_if(qptr, q.end(), p_notalnum);
	if (term_end != q.end()) {
	    string::iterator end2 = find_if(term_end, q.end(), p_notplusminus);
	    if (end2 == q.end() || !isalnum(*end2)) term_end = end2;
	}
	term = q.substr(qptr - q.begin(), term_end - qptr);
	qptr = term_end;
	if (qptr != q.end()) {
	    if (*qptr == '.') {
		// "example.com" should give "exampl" and "com" - need EOF or
		// space after '.' to mean "don't stem"
		qptr++;
		if (qptr == q.end() || isspace(*qptr)) already_stemmed = true;
	    }
	    if (*qptr == '-') {
		if (qptr + 1 != q.end() && isalnum(*(qptr + 1))) {
		    qptr++;
		    pending_token = HYPHEN;
		}
	    }
	}
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
	bool raw_term = (!already_stemmed && !stem_all && !islower(term[0]));
	lowercase_term(term);
	if (raw_term)
	    term = 'R' + term;
	else if (!already_stemmed)
	    term = stemmer->stem_word(term);
	yylval = U(OmQuery(term, 1, termpos++));
	qp->termlist.push_back(term);
	return TERM;
    }
    c = *qptr++;
    switch (c) {
     case '&':
	return AND;
     case '|':
	return OR;
     case '_':
	return HYPHEN;
    }
    /* return single chars */
    return c;                                
}

int
yyerror(const char *s)
{
    throw s;
}

OmQuery
QueryParser::parse_query(const string &q_)
{
    qp = this;
    q = q_;
    pending_token = 0;
    termpos = 1;
    qptr = q.begin();
    if (yyparse() == 1) {
	throw "query failed to parse";
    }
    return query;
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
