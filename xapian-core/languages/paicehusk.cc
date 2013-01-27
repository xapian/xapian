// Alias:paicehusk 

/** @file paicehusk.cc
 *  @Member definitions of class paicehusk.cc.
 */

/* 
 * Copyright (C) 2012 Aarsh Shah
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
*/

/* For details about the paice-husk stemmer ,please go to  http://www.comp.lancs.ac.uk/computing/research/stemming/index.htm */


#include <iostream>
#include <string>
#include <cstring>
#include "paicehusk.h" 


#define FALSE           0
#define TRUE            1
#define EOS           '\0'       /* end-of-string */
#define LAMBDA        "\0"       /* empty string */   
#define IsVowel(c)   ('a'==(c)||'e'==(c)||'i'==(c)||'o'==(c)||'u'==(c))


using namespace std;

//Static DATA Members initialization

char* Xapian::StemPaiceHusk::word_end;
int   Xapian::StemPaiceHusk::word_intact;



// RuleList

static RuleList a_rules[] =
{
      1,      "ia",      LAMBDA,    2,    TRUE,      TRUE,
      0,       "a",      LAMBDA,    1,    TRUE,      TRUE,
      0,      "\0",      LAMBDA,    0,    FALSE,     TRUE
};

static RuleList b_rules[] =
{
      1,      "bb",      LAMBDA,    1,    FALSE,     TRUE,
      0,      "\0",      LAMBDA,    0,    FALSE,     TRUE
};

static RuleList c_rules[] =
{
      3,      "ytic",   "s",        3,    FALSE,     TRUE,
      1,      "ic",     LAMBDA,     2,    FALSE,     FALSE,
      1,      "nc",     "t",        1,    FALSE,     FALSE,
      0,      "\0",     LAMBDA,     0,    FALSE,     TRUE
};

static RuleList d_rules[] =
{
      1,      "dd",     LAMBDA,     1,    FALSE,     TRUE,
      2,      "ied",    "y",        3,    FALSE,     FALSE,
      3,      "ceed",   "ss",       2,    FALSE,     TRUE,
      2,      "eed",    LAMBDA,     1,    FALSE,     TRUE,
      1,      "ed",     LAMBDA,     2,    FALSE,     FALSE,
      3,      "hood",   LAMBDA,     4,    FALSE,     FALSE,
      0,      "\0",     LAMBDA,     0,    FALSE,     TRUE
};

static RuleList e_rules[] =
{
      0,      "e",       LAMBDA,    1,    FALSE,     FALSE,
      0,      "\0",      LAMBDA,    0,    FALSE,     TRUE
};

static RuleList f_rules[] =
{
      3,      "lief",    "v",       1,    FALSE,     TRUE,
      1,      "if",      LAMBDA,    2,    FALSE,     FALSE,
      0,      "\0",      LAMBDA,    0,    FALSE,     TRUE
};

static RuleList g_rules[] =
{
      2,      "ing",     LAMBDA,    3,    FALSE,     FALSE,
      2,      "iag",     "y",       3,    FALSE,     TRUE,
      1,      "ag",      LAMBDA,    2,    FALSE,     FALSE,
      1,      "gg",      LAMBDA,    1,    FALSE,     TRUE,
      0,      "\0",      LAMBDA,    0,    FALSE,     TRUE
};

	

static RuleList h_rules[] =
{
      1,      "th",      LAMBDA,    2,    TRUE,      TRUE,
      4,      "guish",   "ct",      5,    FALSE,     TRUE,
      2,      "ish",     LAMBDA,    3,    FALSE,     FALSE,
      0,      "\0",      LAMBDA,    0,    FALSE,     TRUE
};

static RuleList i_rules[] =
{
      0,      "i",       LAMBDA,    1,    TRUE,      TRUE,
      0,      "i",       "y",       1,    FALSE,     FALSE,
      0,      "\0",      LAMBDA,    0,    FALSE,     TRUE
};

static RuleList j_rules[] =
{
      1,      "ij",      "d",       1,    FALSE,     TRUE,
      2,      "fuj",     "s",       1,    FALSE,     TRUE,
      1,      "uj",      "d",       1,    FALSE,     TRUE,
      1,      "oj",      "d",       1,    FALSE,     TRUE,
      2,      "hej",     "r",       1,    FALSE,     TRUE,
      3,      "verj",    "t",       1,    FALSE,     TRUE,
      3,      "misj",    "t",       2,    FALSE,     TRUE,
      1,      "nj",      "d",       1,    FALSE,     TRUE,
      0,      "j",       "s",       1,    FALSE,     TRUE,
      0,      "\0",      LAMBDA,    0,    FALSE,     TRUE
};

static RuleList k_rules[] =
{
      0,      "\0",      LAMBDA,    0,    FALSE,     TRUE
};

static RuleList l_rules[] =
{
      5,      "ifiabl",  LAMBDA,    6,    FALSE,     TRUE,
      3,      "iabl",    "y",       4,    FALSE,     TRUE,
      2,      "abl",     LAMBDA,    3,    FALSE,     FALSE,
      2,      "ibl",     LAMBDA,    3,    FALSE,     TRUE,
      2,      "bil",     "l",       2,    FALSE,     FALSE,
      1,      "cl",      LAMBDA,    1,    FALSE,     TRUE,
      3,      "iful",    "y",       4,    FALSE,     TRUE,
      2,      "ful",     LAMBDA,    3,    FALSE,     FALSE,
      1,      "ul",      LAMBDA,    2,    FALSE,     TRUE,
      2,      "ial",     LAMBDA,    3,    FALSE,     FALSE,
      2,      "ual",     LAMBDA,    3,    FALSE,     FALSE,
      1,      "al",      LAMBDA,    2,    FALSE,     FALSE,
      1,      "ll",      LAMBDA,    1,    FALSE,     TRUE,
      0,      "\0",      LAMBDA,    0,    FALSE,     TRUE
};

static RuleList m_rules[] =
{
      2,      "ium",     LAMBDA,    3,    FALSE,     TRUE,
      1,      "um",      LAMBDA,    2,    TRUE,      TRUE,
      2,      "ism",     LAMBDA,    3,    FALSE,     FALSE,
      1,      "mm",      LAMBDA,    1,    FALSE,     TRUE,
      0,      "\0",      LAMBDA,    0,    FALSE,     TRUE
};

static RuleList n_rules[] =
{
      3,      "sion",    "j",       4,    FALSE,     FALSE,
      3,      "xion",    "ct",      4,    FALSE,     TRUE,
      2,      "ion",     LAMBDA,    3,    FALSE,     FALSE,
      2,      "ian",     LAMBDA,    3,    FALSE,     FALSE,
      1,      "an",      LAMBDA,    2,    FALSE,     FALSE,
      2,      "een",     LAMBDA,    0,    FALSE,     TRUE,
      1,      "en",      LAMBDA,    2,    FALSE,     FALSE,
      1,      "nn",      LAMBDA,    1,    FALSE,     TRUE,
      0,      "\0",      LAMBDA,    0,    FALSE,     TRUE
};

static RuleList o_rules[] =
{
      0,      "\0",      LAMBDA,    0,    FALSE,     TRUE
};

static RuleList p_rules[] =
{
      3,      "ship",    LAMBDA,    4,    FALSE,     FALSE,
      1,      "pp",      LAMBDA,    1,    FALSE,     TRUE,
      0,      "\0",      LAMBDA,    0,    FALSE,     TRUE
};

static RuleList q_rules[] =
{
      0,      "\0",      LAMBDA,    0,    FALSE,     TRUE
};

static RuleList r_rules[] =
{
      1,      "er",      LAMBDA,    2,    FALSE,     FALSE,
      2,      "ear",     LAMBDA,    0,    FALSE,     TRUE,
      1,      "ar",      LAMBDA,    2,    FALSE,     TRUE,
      1,      "or",      LAMBDA,    2,    FALSE,     FALSE,
      1,      "ur",      LAMBDA,    2,    FALSE,     FALSE,
      1,      "rr",      LAMBDA,    1,    FALSE,     TRUE,
      1,      "tr",      LAMBDA,    1,    FALSE,     FALSE,
      2,      "ier",     "y",       3,    FALSE,     FALSE,
      0,      "\0",      LAMBDA,    0,    FALSE,     TRUE
};

static RuleList s_rules[] =
{
      2,      "ies",     "y",       3,    FALSE,     FALSE,
      2,      "sis",     LAMBDA,    2,    FALSE,     TRUE,
      1,      "is",      LAMBDA,    2,    FALSE,     FALSE,
      3,      "ness",    LAMBDA,    4,    FALSE,     FALSE,
      1,      "ss",      LAMBDA,    0,    FALSE,     TRUE,
      2,      "ous",     LAMBDA,    3,    FALSE,     FALSE,
      1,      "us",      LAMBDA,    2,    TRUE,      TRUE,
      0,      "s",       LAMBDA,    1,    TRUE,      FALSE,
      0,      "s",       LAMBDA,    0,    FALSE,     TRUE,
      0,      "\0",      LAMBDA,    0,    FALSE,     TRUE
};

static RuleList t_rules[] =
{
      5,      "plicat",  "y",       4,    FALSE,     TRUE,
      1,      "at",      LAMBDA,    2,    FALSE,     FALSE,
      3,      "ment",    LAMBDA,    4,    FALSE,     FALSE,
      2,      "ent",     LAMBDA,    3,    FALSE,     FALSE,
      2,      "ant",     LAMBDA,    3,    FALSE,     FALSE,
      3,      "ript",    "b",       2,    FALSE,     TRUE,
      3,      "orpt",    "b",       2,    FALSE,     TRUE,
      3,      "duct",    LAMBDA,    1,    FALSE,     TRUE,
      4,      "sumpt",   LAMBDA,    2,    FALSE,     TRUE,
      3,      "cept",    "iv",      2,    FALSE,     TRUE,
      3,      "olut",    "v",       2,    FALSE,     TRUE,
      3,      "sist",    LAMBDA,    0,    FALSE,     TRUE,
      2,      "ist",     LAMBDA,    3,    FALSE,     FALSE,
      1,      "tt",      LAMBDA,    1,    FALSE,     TRUE,
      0,      "\0",      LAMBDA,    0,    FALSE,     TRUE
};

static RuleList u_rules[] =
{
      2,      "iqu",     LAMBDA,    3,    FALSE,     TRUE,
      2,      "ogu",     LAMBDA,    1,    FALSE,     TRUE,
      0,      "\0",      LAMBDA,    0,    FALSE,     TRUE
};

static RuleList v_rules[] =
{
      2,      "siv",     "j",       3,    FALSE,     FALSE,
      2,      "eiv",     LAMBDA,    0,    FALSE,     TRUE,
      1,      "iv",      LAMBDA,    2,    FALSE,     FALSE,
      0,      "\0",      LAMBDA,    0,    FALSE,     TRUE
};

static RuleList w_rules[] =
{
      0,      "\0",      LAMBDA,    0,    FALSE,     TRUE
};

static RuleList x_rules[] =
{
      0,      "\0",      LAMBDA,    0,    FALSE,     TRUE
};

static RuleList y_rules[] =
{
      2,      "bly",     LAMBDA,    1,    FALSE,     FALSE,
      2,      "ily",     "y",       3,    FALSE,     FALSE,
      2,      "ply",     LAMBDA,    0,    FALSE,     TRUE,
      1,      "ly",      LAMBDA,    2,    FALSE,     FALSE,
      2,      "ogy",     LAMBDA,    1,    FALSE,     TRUE,
      2,      "phy",     LAMBDA,    1,    FALSE,     TRUE,
      2,      "omy",     LAMBDA,    1,    FALSE,     TRUE,
      2,      "opy",     LAMBDA,    1,    FALSE,     TRUE,
      2,      "ity",     LAMBDA,    3,    FALSE,     FALSE,
      2,      "ety",     LAMBDA,    3,    FALSE,     FALSE,
      2,      "lty",     LAMBDA,    2,    FALSE,     TRUE,
      4,      "istry",   LAMBDA,    5,    FALSE,     TRUE,
      2,      "ary",     LAMBDA,    3,    FALSE,     FALSE,
      2,      "ory",     LAMBDA,    3,    FALSE,     FALSE,
      2,      "ify",     LAMBDA,    3,    FALSE,     TRUE,
      2,      "ncy",     "t",       2,    FALSE,     FALSE,
      2,      "acy",     LAMBDA,    3,    FALSE,     FALSE,
      0,      "\0",      LAMBDA,    0,    FALSE,     TRUE
};

static RuleList z_rules[] =
{
      1,      "iz",      LAMBDA,    2,    FALSE,     FALSE,
      1,      "yz",      "s",       1,    FALSE,     TRUE,
      0,      "\0",      LAMBDA,    0,    FALSE,     TRUE
};



//Private Member functions

int Xapian::StemPaiceHusk::Acceptance_Test(char *word) 
{
    char *start; 
    if (IsVowel(*word)) {               
        if (1<strlen(word))           
             return(TRUE);
    }
    else {                            
           if (2<strlen(word)) {          
               for (start = word;*start != EOS;start++){        
                   if (IsVowel( *start )||'y'==*start)  
                       return(TRUE);
               }
           }
         }
    return(FALSE);
} 




int Xapian::StemPaiceHusk::Implement_Rules(char *word,RuleList *rule )         
{
    int word_length = strlen(word); 
    for (/* rules passed in */ ;*(rule->old_end);rule++) {
         if (rule->old_offset<word_length) {
              char *ending = word_end - rule->old_offset;
              if (0==strcmp(ending,rule->old_end))
                  if (!rule->intact_flag||(rule->intact_flag && word_intact)) {
                      char stemmed_word[word_length+1];
                      (void)strcpy( stemmed_word, word );
                      (void)strcpy( stemmed_word + word_length - rule->replace_length,
                      rule->new_end );
                      if (Acceptance_Test(stemmed_word)) {                 
                          (void)strcpy( word, stemmed_word );
                          word_end = word + strlen(word) - 1;
                          word_intact = FALSE;
                          return ( rule->terminate_flag );
                      }
                  }
          }
    }
    return(TRUE);  
} 



int Xapian::StemPaiceHusk::PaiceStem(char *word)      
{
    int stop = TRUE;      
    for (word_end=word;*word_end!= EOS;word_end++)
          if (!isalpha(*word_end))
	       return( FALSE );
          word_end--;                     
      
    word_intact = TRUE;
    do                  
          switch(*word_end) {
                   case 'a': stop = Implement_Rules( word, a_rules );
                             break;
                   case 'b': stop = Implement_Rules( word, b_rules );
                             break;
                   case 'c': stop = Implement_Rules( word, c_rules );
                             break;
                   case 'd': stop = Implement_Rules( word, d_rules );
                             break;
                   case 'e': stop = Implement_Rules( word, e_rules );
                             break;
                   case 'f': stop = Implement_Rules( word, f_rules );
                             break;
       	   	   case 'g': stop = Implement_Rules( word, g_rules );
                             break;
                   case 'h': stop = Implement_Rules( word, h_rules );
                             break;
                   case 'i': stop = Implement_Rules( word, i_rules );
                             break;
                   case 'j': stop = Implement_Rules( word, j_rules );
                             break;
                   case 'k': stop = Implement_Rules( word, k_rules );
                             break;
                   case 'l': stop = Implement_Rules( word, l_rules );
                             break;
                   case 'm': stop = Implement_Rules( word, m_rules );
                             break;
                   case 'n': stop = Implement_Rules( word, n_rules );
                             break;
                   case 'o': stop = Implement_Rules( word, o_rules );
                             break;
                   case 'p': stop = Implement_Rules( word, p_rules );
                             break;
                   case 'q': stop = Implement_Rules( word, q_rules );
                             break;
                   case 'r': stop = Implement_Rules( word, r_rules );
                             break;
                   case 's': stop = Implement_Rules( word, s_rules );
                             break;
                   case 't': stop = Implement_Rules( word, t_rules );
                             break;
                   case 'u': stop = Implement_Rules( word, u_rules );
                             break;
                   case 'v': stop = Implement_Rules( word, v_rules );
                             break;
                   case 'w': stop = Implement_Rules( word, w_rules );
                             break;
                   case 'x': stop = Implement_Rules( word, x_rules );
                             break;
                   case 'y': stop = Implement_Rules( word, y_rules );
                             break;
                   case 'z': stop = Implement_Rules( word, z_rules );
                             break;
                   default : stop = TRUE;   
                             break;
          }  
    while (!stop);   	
    return (TRUE); 
} 

//PUBLIC MEMBER FUNCTIONS 

Xapian::StemPaiceHusk::~StemPaiceHusk()
{

} 

string Xapian::StemPaiceHusk::get_description() const
{
    return "PaiceHusk Stemmer"; 
}

string Xapian::StemPaiceHusk::operator()(const string & word)
{
    char *newword=new char[word.size()+1];
    int j;
    for (j=0;j<word.size();j++)
          newword[j]=word[j];
	  newword[j]='\0';
	
	if (PaiceStem(newword)) {
		const char *stem_word=newword;
		string final_stem=stem_word;
		delete [] newword;
		return final_stem;	        	
	}
	else
		
		return word;
}


