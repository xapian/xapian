
/*  A Bison parser, made from /home/richard/stuff/Working/xapian/om-examples/omega/parsequery.yy
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define	TERM	257
#define	HYPHEN	258
#define	OR	259
#define	XOR	260
#define	NOT	261
#define	AND	262
#define	NEAR	263

#line 23 "parsequery.yy"

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
    
#ifndef YYSTYPE
#define YYSTYPE int
#endif
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		38
#define	YYFLAG		-32768
#define	YYNTBASE	17

#define YYTRANSLATE(x) ((unsigned)(x) <= 263 ? yytranslate[x] : 23)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,    14,     2,     2,     2,     2,     2,    12,
    13,     2,    10,     2,    11,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    15,     2,    16,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     3,     5,     9,    13,    17,    21,    25,    27,
    30,    34,    38,    40,    44,    48,    50,    54,    56,    59,
    63
};

static const short yyrhs[] = {    -1,
    18,     0,    19,     0,    18,     8,    18,     0,    18,     5,
    18,     0,    18,     7,    18,     0,    18,     6,    18,     0,
    12,    18,    13,     0,    20,     0,    19,    20,     0,    19,
    10,    20,     0,    19,    11,    20,     0,     3,     0,     3,
     9,     3,     0,    14,    21,    14,     0,    22,     0,    15,
    21,    16,     0,     3,     0,    21,     3,     0,     3,     4,
     3,     0,    22,     4,     3,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    67,    68,    71,    92,    93,    94,    95,    96,    99,   100,
   103,   104,   107,   108,   113,   115,   117,   121,   122,   125,
   126
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","TERM","HYPHEN",
"OR","XOR","NOT","AND","NEAR","'+'","'-'","'('","')'","'\\\"'","'{'","'}'","input",
"exp","prob","term","phrase","hypphr", NULL
};
#endif

static const short yyr1[] = {     0,
    17,    17,    18,    18,    18,    18,    18,    18,    19,    19,
    19,    19,    20,    20,    20,    20,    20,    21,    21,    22,
    22
};

static const short yyr2[] = {     0,
     0,     1,     1,     3,     3,     3,     3,     3,     1,     2,
     3,     3,     1,     3,     3,     1,     3,     1,     2,     3,
     3
};

static const short yydefact[] = {     1,
    13,     0,     0,     0,     2,     3,     9,    16,     0,     0,
     0,    18,     0,     0,     0,     0,     0,     0,     0,     0,
    10,     0,    20,    14,     8,    19,    15,    17,     5,     7,
     6,     4,    11,    12,    21,     0,     0,     0
};

static const short yydefgoto[] = {    36,
     5,     6,     7,    13,     8
};

static const short yypact[] = {     9,
    -3,     9,     0,     0,     2,    17,-32768,     7,    23,    26,
    32,-32768,    22,     1,     9,     9,     9,     9,    19,    19,
-32768,    27,-32768,-32768,-32768,-32768,-32768,-32768,    -6,    -6,
    -6,-32768,-32768,-32768,-32768,    35,    41,-32768
};

static const short yypgoto[] = {-32768,
    -2,-32768,    -1,    38,-32768
};


#define	YYLAST		45


static const short yytable[] = {    11,
     9,    18,    12,    26,    21,    10,    15,    16,    17,    18,
    22,     1,    29,    30,    31,    32,    28,    33,    34,     1,
     2,     1,     3,     4,    26,    23,    19,    20,    24,    35,
     3,     4,     3,     4,    37,    27,    15,    16,    17,    18,
    38,    14,     0,     0,    25
};

static const short yycheck[] = {     2,
     4,     8,     3,     3,     6,     9,     5,     6,     7,     8,
     4,     3,    15,    16,    17,    18,    16,    19,    20,     3,
    12,     3,    14,    15,     3,     3,    10,    11,     3,     3,
    14,    15,    14,    15,     0,    14,     5,     6,     7,     8,
     0,     4,    -1,    -1,    13
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison/bison.simple"
/* This file comes from bison-1.28.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 217 "/usr/share/bison/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
int yyparse (void *);
#else
int yyparse (void);
#endif
#endif

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
  int yyfree_stacks = 0;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 1:
#line 67 "parsequery.yy"
{ query = OmQuery(); ;
    break;}
case 2:
#line 68 "parsequery.yy"
{ query = yyvsp[0].q; ;
    break;}
case 3:
#line 71 "parsequery.yy"
{
			    OmQuery q = yyvsp[0].q;
			    if (yyvsp[0].love.size()) {
				q = OmQuery(OmQuery::OP_AND_MAYBE,
					    OmQuery(OmQuery::OP_AND,
						    yyvsp[0].love.begin(),
						    yyvsp[0].love.end()),
					    q);
			    }
			    if (yyvsp[0].hate.size()) {
				if (!q.is_defined()) {
				    // FIXME: barf
				}
				q = OmQuery(OmQuery::OP_AND_NOT,
					    q,
					    OmQuery(OmQuery::OP_OR,
						    yyvsp[0].hate.begin(),
						    yyvsp[0].hate.end()));
			    }
			    yyval = q;
			;
    break;}
case 4:
#line 92 "parsequery.yy"
{ yyval = U(OmQuery(OmQuery::OP_AND, yyvsp[-2].q, yyvsp[0].q)); ;
    break;}
case 5:
#line 93 "parsequery.yy"
{ yyval = U(OmQuery(OmQuery::OP_OR, yyvsp[-2].q, yyvsp[0].q)); ;
    break;}
case 6:
#line 94 "parsequery.yy"
{ yyval = U(OmQuery(OmQuery::OP_AND_NOT, yyvsp[-2].q, yyvsp[0].q)); ;
    break;}
case 7:
#line 95 "parsequery.yy"
{ yyval = U(OmQuery(OmQuery::OP_XOR, yyvsp[-2].q, yyvsp[0].q)); ;
    break;}
case 8:
#line 96 "parsequery.yy"
{ yyval = yyvsp[-1]; ;
    break;}
case 9:
#line 99 "parsequery.yy"
{ yyval = yyvsp[0]; ;
    break;}
case 10:
#line 100 "parsequery.yy"
{ yyval = U(OmQuery(default_op, yyvsp[-1].q, yyvsp[0].q));
	                  yyval.love = yyvsp[-1].love;
	                  yyval.hate = yyvsp[-1].hate; ;
    break;}
case 11:
#line 103 "parsequery.yy"
{ yyval = yyvsp[-2]; yyval.love.push_back(yyvsp[0].q); ;
    break;}
case 12:
#line 104 "parsequery.yy"
{ yyval = yyvsp[-2]; yyval.hate.push_back(yyvsp[0].q); ;
    break;}
case 13:
#line 107 "parsequery.yy"
{ yyval = yyvsp[0]; ;
    break;}
case 14:
#line 108 "parsequery.yy"
{ vector<OmQuery> v;
	                  v.push_back(yyvsp[-2].q);
	                  v.push_back(yyvsp[0].q);
			  yyval = U(OmQuery(OmQuery::OP_NEAR, v.begin(), v.end()));
			  yyval.q.set_window(11); ;
    break;}
case 15:
#line 113 "parsequery.yy"
{ yyval = U(OmQuery(OmQuery::OP_PHRASE, yyvsp[-1].v.begin(), yyvsp[-1].v.end()));
			  yyval.q.set_window(yyvsp[-1].v.size()); ;
    break;}
case 16:
#line 115 "parsequery.yy"
{ yyval = U(OmQuery(OmQuery::OP_PHRASE, yyvsp[0].v.begin(), yyvsp[0].v.end()));
			  yyval.q.set_window(yyvsp[0].v.size()); ;
    break;}
case 17:
#line 117 "parsequery.yy"
{ yyval = U(OmQuery(OmQuery::OP_NEAR, yyvsp[-1].v.begin(), yyvsp[-1].v.end()));
			  yyval.q.set_window(yyvsp[-1].v.size()); ;
    break;}
case 18:
#line 121 "parsequery.yy"
{ yyval.v.push_back(yyvsp[0].q); ;
    break;}
case 19:
#line 122 "parsequery.yy"
{ yyval = yyvsp[-1]; yyval.v.push_back(yyvsp[0].q); ;
    break;}
case 20:
#line 125 "parsequery.yy"
{ yyval.v.push_back(yyvsp[-2].q); yyval.v.push_back(yyvsp[0].q); ;
    break;}
case 21:
#line 126 "parsequery.yy"
{ yyval = yyvsp[-2]; yyval.v.push_back(yyvsp[0].q); ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "/usr/share/bison/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 1;
}
#line 129 "parsequery.yy"


static string::size_type qptr;
static int pending_token;
static om_termpos termpos;
static bool stem, stem_all;
OmStem *stemmer;

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
