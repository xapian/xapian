/* phrasepostlist.h: Return only items where terms are near or form a phrase
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

#ifndef OM_HGUARD_PHRASEPOSTLIST_H
#define OM_HGUARD_PHRASEPOSTLIST_H

#include "database.h"
#include "selectpostlist.h"
#include "omdebug.h"

class PosListBuffer;

/** Parent class for NearPostList and PhrasePostList
 */
class NearOrPhrasePostList : public SelectPostList {
    protected:
        om_termpos window;
	vector<PostList *> terms;

    	bool test_doc();
        virtual bool do_test(vector<PosListBuffer> &plists, om_termcount i,
			     om_termcount min, om_termcount max) = 0;
    public:
	string intro_term_description() const;

        NearOrPhrasePostList(PostList *source, om_termpos window_,
			     vector<PostList *> terms_)
	: SelectPostList(source)
        {
	    window = window_;
	    terms = terms_;
	}
};

inline string
NearOrPhrasePostList::intro_term_description() const
{
    return "(NearOrPhrase " + om_inttostring(window) + " "
	   + source->intro_term_description() + ")";
}


/** A postlist comprising several postlists NEARed together.
 *
 *  This postlist returns a posting if and only if it is in all of the
 *  sub-postlists and all the terms occur within a specified distance of
 *  each other somewhere in the document.  The weight for a posting is the
 *  sum of the weights of the sub-postings.
 */
class NearPostList : public NearOrPhrasePostList {
    private:
        bool do_test(vector<PosListBuffer> &plists, om_termcount i,
		     om_termcount min, om_termcount max);
    public:
	string intro_term_description() const;

        NearPostList(PostList *source, om_termpos window_,
		     vector<PostList *> terms_)
	: NearOrPhrasePostList(source, window_, terms_) { }
};

inline string
NearPostList::intro_term_description() const
{
    return "(Near " + om_inttostring(window) + " "
	   + source->intro_term_description() + ")";
}


/** A postlist comprising several postlists PHRASEd together.
 *
 *  This postlist returns a posting if and only if it is in all of the
 *  sub-postlists and all the terms occur IN THE GIVEN ORDER within a
 *  specified distance of each other somewhere in the document.  The weight
 *  for a posting is the sum of the weights of the sub-postings.
 */
class PhrasePostList : public NearOrPhrasePostList {
    private:
        bool do_test(vector<PosListBuffer> &plists, om_termcount i,
		     om_termcount mine, om_termcount max);
    public:
	string intro_term_description() const;

        PhrasePostList(PostList *source, om_termpos window_,
		       vector<PostList *> terms_)
	: NearOrPhrasePostList(source, window_, terms_) { }
};

inline string
PhrasePostList::intro_term_description() const
{
    return "(Phrase " + om_inttostring(window) + " "
	   + source->intro_term_description() + ")";
}

#endif /* OM_HGUARD_PHRASEPOSTLIST_H */
