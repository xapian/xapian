/* parsequery.yy: parser for query strings
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
	U(OmQuery q) : q(q) { }
	U() { }
};

#define YYSTYPE U

int yyerror(const char *s);
int yylex();
%}

/* BISON Declarations */
%token TERM
%left OR XOR NOT
%left AND

/* Grammar follows */
%%
input:	  { cout << "no query\n"; }
	| exp { cout << $1.q << endl; }
;

exp:	  terms			{ $$ = $1; }
	| exp AND exp		{ $$ = U(OmQuery(OM_MOP_AND, $1.q, $3.q)); }
	| exp OR exp		{ $$ = U(OmQuery(OM_MOP_OR, $1.q, $3.q)); }
	| exp NOT exp		{ $$ = U(OmQuery(OM_MOP_AND_NOT, $1.q, $3.q)); }
	| exp XOR exp		{ $$ = U(OmQuery(OM_MOP_XOR, $1.q, $3.q)); }
	| '"' phrase '"'	{ $$ = U(OmQuery(OM_MOP_PHRASE, $2.v.begin(), $2.v.end(), $2.v.size())); }
;

terms:	  TERM			{ $$ = $1; }
	| TERM terms		{ $$ = U(OmQuery(OM_MOP_OR, $1.q, $2.q)); }
;

phrase:	  TERM			{ $$.v.push_back($1.q); }
	| phrase TERM		{ $$ = $1; $$.v.push_back($2.q); }
;
%%

/* Lexical analyzer returns a string containing a term name
   on the stack and the token TERM, a token for an operator,
   or the ASCII character read otherwise.  Skips all blanks
   and tabs, returns 0 for EOF. */

#include <ctype.h>

static om_termpos termpos = 1;

yylex()
{
    int c;

    /* skip white space  */
    while (isspace((c = getchar())))  
	;
    /* process terms */
    if (isalnum(c)) {
	string term;
	term = char(c);
	c = getchar();
	while (isalnum(c)) {
	    term += char(c);
	    c = getchar();
	}
	ungetc(c, stdin);
	if (term == "AND") {
	    return AND;
        } else if (term == "OR") {
	    return OR;
        } else if (term == "NOT") {
	    return NOT;
        } else if (term == "XOR") {
	    return XOR;
        }
	yylval = U(OmQuery(term, 1, termpos++));
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

yyerror(const char *s)  /* Called by yyparse on error */
{
    printf("%s\n", s);
}

int main()
{
    yyparse();
    return 0;
}
