
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

HEXDIGIT	[0-9a-fA-F]
TCHAR		{HEXDIGIT}{HEXDIGIT}
DIGIT		[0-9]

TERM		%T{TCHAR}*,{DIGIT}*,{DIGIT}*

NULL_QUERY	%N

BOOL_FLAG	%B

OP_BRA		%\(
OP_KET		%\)
%%

{TERM}		{
		    qt.type = querytok::TERM;
		    qt.wqf = 0;
		    qt.term_pos = 0;
		    qt.tname.erase();
		    char *p = yytext + 2; // skip %T
		    while (*p && *p != ',') {
		        char high = *p++;
			if (!isxdigit(high)) {
			    qt.type = querytok::ERROR;
			    return qt;
			}
			char low = *p++;
			if (!isxdigit(low)) {
			    qt.type = querytok::ERROR;
			    return qt;
			}
			qt.tname += hextochar(high, low);
		    }
		    if (*p != ',') {
		    	qt.type = querytok::ERROR;
			return qt;
		    }
		    p++;
		    while (isdigit(*p)) {
		        qt.wqf *= 10;
			qt.wqf += *p - '0';
			++p;
		    }
		    if (*p != ',') {
		    	qt.type = querytok::ERROR;
			return qt;
		    }
		    p++;
		    while (isdigit(*p)) {
		        qt.term_pos *= 10;
			qt.term_pos += *p - '0';
			++p;
		    }
		    if (*p != 0) {
		        qt.type = querytok::ERROR;
			return qt;
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

{OP_OR}	{
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
		    char *p = yytext + 5; // skip %near

		    qt.window = 0;
		    while (isdigit(*p)) {
		        qt.window *= 10;
			qt.window += *p - '0';
			++p;
		    }
		    return qt;
		}

{OP_PHRASE}	{
		    qt.type = querytok::OP_PHRASE;
		    char *p = yytext + 7; // skip %phrase

		    qt.window = 0;
		    while (isdigit(*p)) {
		        qt.window *= 10;
			qt.window += *p - '0';
			++p;
		    }
		    return qt;
		}

{NULL_QUERY}	{
		    qt.type = querytok::NULL_QUERY;
		    return qt;
		}

{BOOL_FLAG}	{
		    qt.type = querytok::BOOL_FLAG;
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

void qfs_start(string text)
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
