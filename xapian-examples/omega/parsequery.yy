/* parsequery.yy: parser for omega query strings
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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
#include <string>
#include <om/om.h>
#include <stdio.h>
#include <ctype.h>

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
				q = OmQuery(OmQuery::OP_AND_MAYBE,
					    OmQuery(OmQuery::OP_AND,
						    $1.love.begin(),
						    $1.love.end()),
					    q);
			    }
			    if ($1.hate.size()) {
				if (!q.is_defined()) {
				    // FIXME: barf
				}
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

prob:	  term		{ $$ = $1; }
	| prob term	{ $$ = U(OmQuery(OmQuery::OP_OR, $1.q, $2.q));
	                  $$.love = $1.love;
	                  $$.hate = $1.hate; }			  
	| prob '+' term	{ $$ = $1; $$.love.push_back($3.q); }
	| prob '-' term	{ $$ = $1; $$.hate.push_back($3.q); }
;

term:	  TERM		{ $$ = $1; }
	| TERM NEAR TERM{ vector<OmQuery> v;
	                  v.push_back($1.q);
	                  v.push_back($3.q);
			  $$ = U(OmQuery(OmQuery::OP_NEAR, v.begin(), v.end(), 11)); }
	| '"' phrase '"'{ $$ = U(OmQuery(OmQuery::OP_PHRASE, $2.v.begin(), $2.v.end(), $2.v.size())); }
	| hypphr        { $$ = U(OmQuery(OmQuery::OP_PHRASE, $1.v.begin(), $1.v.end(), $1.v.size())); }
	| '{' phrase '}'{ $$ = U(OmQuery(OmQuery::OP_NEAR, $2.v.begin(), $2.v.end(), $2.v.size())); }
;

phrase:	  TERM		{ $$.v.push_back($1.q); }
	| phrase TERM	{ $$ = $1; $$.v.push_back($2.q); }
;

hypphr:   TERM HYPHEN TERM	{ $$.v.push_back($1.q); $$.v.push_back($3.q); }
	| hypphr HYPHEN TERM	{ $$ = $1; $$.v.push_back($3.q); }
;

%%

static string::size_type qptr;
static int pending_token;
static om_termpos termpos;
static bool stem, stem_all;
static OmStem *stemmer;

void
QueryParser::set_stemming_options(const string &lang, bool stem_all_)
{
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
   if (qptr >= q.size()) return EOF;
   return q[qptr++];
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
    qptr = q.find_first_not_of(" \t\n\r\f\v", qptr);
    if (qptr == string::npos) return 0;

    /* process terms */
    if (isalnum(q[qptr])) {
	string term;
	bool stem_term = stem;
	string::size_type term_end;
	term_end = q.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				       "abcdefghijklmnopqrstuvwxyz"
				       "0123456789", qptr);
	if (term_end != string::npos) {
	    string::size_type end2 = q.find_first_not_of("+-", term_end);
	    if (end2 == string::npos || !isalnum(q[end2])) term_end = end2;
	}
	term = q.substr(qptr, term_end - qptr);
	qptr = term_end;
	if (qptr != string::npos) {
	    if (q[qptr] == '.') {
		// "example.com" should give "exampl" and "com" - need EOF or
		// space after '.' to mean "don't stem"
		qptr++;
		if (qptr == q.size() || isspace(q[qptr])) stem_term = false;
	    }
	    if (q[qptr] == '-') {
		if (qptr + 1 != q.size() && isalnum(q[qptr + 1])) {
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
	if (stem_term) term = stemmer->stem_word(term);
	yylval = U(OmQuery(term, 1, termpos++));
	qp->termlist.push_back(term);
	qp->termset.insert(term);
	return TERM;
    }
    c = q[qptr++];
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
    qptr = 0;
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
