// util.C
//
// (c) 2001 Amir Michail (amir@users.sourceforge.net)

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

#include <om/om.h>
#include "util.h"

void lowercase_term(om_termname &term)
{
    om_termname::iterator i = term.begin();
    while(i != term.end()) {
        *i = tolower(*i);
        i++;
    }
}

void split (const string & text, const string & separators, list<string> & words)
{
   int n = text.length();
   int start, stop;

   start = text.find_first_not_of(separators);
   while ((start >= 0) && (start < n)) {
     stop = text.find_first_of(separators, start);
     if ((stop < 0) || (stop > n)) {
       stop = n;
     }
     string word = text.substr(start, stop - start);
     words.push_back(word);
     start = text.find_first_not_of(separators, stop+1);
   }
}
