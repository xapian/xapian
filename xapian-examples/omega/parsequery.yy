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

static int yyerror(const char *s);
static int yylex();

static string::const_iterator q_ptr;

#include "omega.h"
#include "query.h"
%}

/* BISON Declarations */
%token TERM
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
				q = OmQuery(OM_MOP_AND_MAYBE,
					    OmQuery(OM_MOP_AND,
						    $1.love.begin(),
						    $1.love.end()),
					    q);
			    }
			    if ($1.hate.size()) {
				if (!q.is_defined()) {
				    // FIXME: barf
				}
				q = OmQuery(OM_MOP_AND_NOT,
					    q,
					    OmQuery(OM_MOP_OR,
						    $1.hate.begin(),
						    $1.hate.end()));
			    }
			    $$ = q;
			}
	| exp AND exp	{ $$ = U(OmQuery(OM_MOP_AND, $1.q, $3.q)); }
	| exp OR exp	{ $$ = U(OmQuery(OM_MOP_OR, $1.q, $3.q)); }
	| exp NOT exp	{ $$ = U(OmQuery(OM_MOP_AND_NOT, $1.q, $3.q)); }
	| exp XOR exp	{ $$ = U(OmQuery(OM_MOP_XOR, $1.q, $3.q)); }
	| '(' exp ')'	{ $$ = $2; }
;

prob:	  term		{ $$ = $1; }
	| prob term	{ $$ = U(OmQuery(OM_MOP_OR, $1.q, $2.q));
	                  $$.love = $1.love;
	                  $$.hate = $1.hate; }			  
	| prob '+' term	{ $$ = $1; $$.love.push_back($3.q); }
	| prob '-' term	{ $$ = $1; $$.hate.push_back($3.q); }
;

term:	  TERM		{ $$ = $1; }
	| TERM NEAR TERM{ vector<OmQuery> v;
	                  v.push_back($1.q);
	                  v.push_back($3.q);
			  $$ = U(OmQuery(OM_MOP_NEAR, v.begin(), v.end(), 11)); }
	| '"' phrase '"'{ $$ = U(OmQuery(OM_MOP_PHRASE, $2.v.begin(), $2.v.end(), $2.v.size())); }
/*	| dashphr       { $$ = U(OmQuery(OM_MOP_PHRASE, $2.v.begin(), $2.v.end(), $2.v.size())); } */
	| '{' phrase '}'{ $$ = U(OmQuery(OM_MOP_NEAR, $2.v.begin(), $2.v.end(), $2.v.size())); }
;

phrase:	  TERM		{ $$.v.push_back($1.q); }
	| phrase TERM	{ $$ = $1; $$.v.push_back($2.q); }
;

/*
dashphr:  TERM '-' TERM	{ $$.v.push_back($1.q); $$.v.push_back($3.q); }
	| dashphr '-' TERM
			{ $$ = $1; $$.v.push_back($3.q); }
;
*/

%%

/* Lexical analyzer returns a string containing a term name
   on the stack and the token TERM, a token for an operator,
   or the ASCII character read otherwise.  Skips all blanks
   and tabs, returns 0 for EOF. */

#include <ctype.h>

static om_termpos termpos = 1;

static inline int
next_char()
{
   if (q_ptr == raw_prob.end()) return EOF;
   return *q_ptr++;
}

static OmStem stemmer("english");

static bool stem, stem_all;

// FIXME: allow domain:uk in query...
// don't allow + and & in term then ?
// or allow +&.-_ ?
// domain/site/language/host ?

static int
yylex()
{
    int c;

    /* skip white space  */
    while (isspace((c = next_char())))
	;
    /* process terms */
    if (isalnum(c)) {
	string term;
	bool stem_term = stem;
	term = char(c);
	c = next_char();
	while (isalnum(c)) {
	    term += char(c);
	    c = next_char();
	}
	if (c == '.') {
	    // "example.com" should give "exampl" and "com" - need EOF or
	    // space after '.' to mean "don't stem"
	    c = next_char();
	    if (c == EOF || isspace(c)) stem_term = false;	    
	}
	if (c != EOF) q_ptr--;
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
	if (stem_term) term = stemmer.stem_word(term);
	yylval = U(OmQuery(term, 1, termpos++));
	new_terms_list.push_back(term);
	new_terms.insert(term);
	return TERM;
    }
    switch (c) {
     case EOF:
	return 0;
     case '&':
	return AND;
     case '|':
	return OR;
    }
    /* return single chars */
    return c;                                
}

static int
yyerror(const char *s)
{
    throw s;
}

void
parse_prob()
{
    stem = !atoi(option["no_stem"].c_str());
    // stem capitalised words too -- needed for EuroFerret
    stem_all = atoi(option["all_stem"].c_str());
    q_ptr = raw_prob.begin();
    yyparse();
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
