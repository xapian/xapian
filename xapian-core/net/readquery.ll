
%{

#include "readquery.h"
#include <stdio.h>

#define YY_DECL querytok yylex()

querytok qt;
%}

%option prefix="omquery"
%option noyywrap
%option stdinit
%option never-interactive
%option nounput

OP_AND		%and
OP_OR		%or
OP_FILTER	%filter
OP_ANDMAYBE	%andmaybe
OP_ANDNOT	%andnot
OP_XOR		%xor
OP_NEAR		%near{DIGIT}{DIGIT}*
OP_PHRASE	%phrase{DIGIT}{DIGIT}*
OP_WEIGHT_CUTOFF	%wtcutoff{DIGIT}{DIGIT}*\.{DIGIT}{DIGIT}*
OP_PERCENT_CUTOFF	%pctcutoff{DIGIT}{DIGIT}*\.{DIGIT}{DIGIT}*
OP_ELITE_SET	%eliteset{DIGIT}{DIGIT}*

TCHAR		[^ \n]
DIGIT		[0-9]

TERM		%T{TCHAR}*\ {DIGIT}{DIGIT}*(,{DIGIT}{DIGIT}*)?

NULL_QUERY	%N

QUERY_LEN	%L{DIGIT}{DIGIT}*

OP_BRA		%\(
OP_KET		%\)
%%

{TERM}		{
		    qt.type = querytok::TERM;
		    qt.tname.erase();
		    const char *p = yytext + 2; // skip %T
		    const char *q = strchr(p, ' ');
		    Assert(q != NULL);
		    qt.tname = decode_tname(std::string(p, q - p));
		    {		    
			char *tmp; // avoid compiler warning
			qt.term_pos = strtol(q + 1, &tmp, 10);
			p = tmp;
		    }
		    if (*p == ',') {
		        qt.wqf = atoi(p + 1);
		    } else {
		        Assert(*p == '\0');
			qt.wqf = 1;
		    }
		    return qt;
		}

{OP_BRA}	{
		    qt.type = querytok::OP_BRA;
		    return qt;
		}

{OP_KET}	{
		    qt.type = querytok::OP_KET;
		    return qt;
		}

{OP_AND}	{
		    qt.type = querytok::OP_AND;
		    return qt;
		}

{OP_OR}		{
		    qt.type = querytok::OP_OR;
		    return qt;
		}

{OP_FILTER}	{
		    qt.type = querytok::OP_FILTER;
		    return qt;
		}

{OP_ANDMAYBE}	{
		    qt.type = querytok::OP_ANDMAYBE;
		    return qt;
		}

{OP_ANDNOT}	{
		    qt.type = querytok::OP_ANDNOT;
		    return qt;
		}

{OP_XOR}	{
		    qt.type = querytok::OP_XOR;
		    return qt;
		}

{OP_NEAR}	{
		    qt.type = querytok::OP_NEAR;
		    qt.window = atoi(yytext + 5); // skip %near
		    return qt;
		}

{OP_PHRASE}	{
		    qt.type = querytok::OP_PHRASE;
		    qt.window = atoi(yytext + 7); // skip %phrase
		    return qt;
		}

{OP_WEIGHT_CUTOFF}	{
		    qt.type = querytok::OP_WEIGHT_CUTOFF;
		    qt.cutoff = atof(yytext + 9); // skip %wtcutoff
		    return qt;
		}

{OP_PERCENT_CUTOFF}	{
		    qt.type = querytok::OP_PERCENT_CUTOFF;
		    qt.cutoff = atof(yytext + 10); // skip %pctcutoff
		    return qt;
		}

{OP_ELITE_SET}	{
		    qt.type = querytok::OP_ELITE_SET;
		    qt.elite_set_size = atoi(yytext + 9); // skip %eliteset
		    return qt;
		}

{NULL_QUERY}	{
		    qt.type = querytok::NULL_QUERY;
		    return qt;
		}

{QUERY_LEN}	{
		    qt.type = querytok::QUERY_LEN;
		    qt.qlen = atoi(yytext + 2);
		    return qt;
		}

[ \t]*		/* skip whitespace */

.		{
		    qt.type = querytok::ERROR;
		    return qt;
		}

<<EOF>>		{
		    qt.type = querytok::END;
		    return qt;
		}

%%

static YY_BUFFER_STATE qfs_yystate;

void qfs_start(std::string text)
{
    qfs_yystate = yy_scan_string(text.c_str());
}

querytok qfs_gettok()
{
    return yylex();
}

void qfs_end()
{
    yy_delete_buffer(qfs_yystate);
}
