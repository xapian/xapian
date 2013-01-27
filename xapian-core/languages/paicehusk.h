/** @file paicehusk.h */
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

/* For details about the PaiceHusk Stemmer please go to  http://www.comp.lancs.ac.uk/computing/research/stemming/index.htm */ 


#include <string>
#include <xapian/stem.h>
#include <cstring>   

typedef struct 
{
    int old_offset;          /* from end of word to start of suffix */
    const char *old_end;     /* suffix replaced */
    const char *new_end;     /* suffix replacement */
    int replace_length;      /* length of old suffix to replace */
    int intact_flag;         /* use rule only if original word */
    int terminate_flag;      /* continue stemming or quit flag */
} RuleList;

namespace Xapian {

class StemPaiceHusk:public Xapian::StemImplementation {
    static char *word_end;         
    static int word_intact;
    int Implement_Rules(char *word, RuleList *rule);
    int Acceptance_Test(char *word);
    int PaiceStem(char *word);
  
  public:
    std::string operator()(const std::string & word);
    std::string get_description() const;
    ~StemPaiceHusk();	
	    		
};

}
