/** @file
 * @brief A termlist containing all words which are spelling targets.
 */
/* Copyright (C) 2005,2008,2009,2010,2011,2017 Olly Betts
 * Copyright (C) 2007 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_HONEY_SPELLINGWORDSLIST_H
#define XAPIAN_INCLUDED_HONEY_SPELLINGWORDSLIST_H

#include "backends/alltermslist.h"
#include "honey_spelling.h"
#include "honey_cursor.h"

class HoneyDatabase;

class HoneySpellingWordsList : public AllTermsList {
    /// Copying is not allowed.
    HoneySpellingWordsList(const HoneySpellingWordsList&);

    /// Assignment is not allowed.
    void operator=(const HoneySpellingWordsList&);

    /// Keep a reference to our database to stop it being deleted.
    Xapian::Internal::intrusive_ptr<const HoneyDatabase> database;

    /** A cursor which runs through the spelling table reading termnames from
     *  the keys.
     */
    HoneyCursor* cursor;

    /** The term frequency of the term at the current position.
     *
     *  If this value is zero, then we haven't read the term frequency or
     *  collection frequency for the current term yet.  We need to call
     *  read_termfreq() to read these.
     */
    mutable Xapian::termcount termfreq;

    /// Read and cache the term frequency.
    void read_termfreq() const;

  public:
    HoneySpellingWordsList(const HoneyDatabase* database_, HoneyCursor* cursor_)
	    : database(database_), cursor(cursor_), termfreq(0) {
	// Set the cursor to its end to signal we haven't started yet.  Then
	// if the first action is next() we can move the cursor to the first
	// word with:
	//
	// cursor.find_entry_ge(string(1, KEY_PREFIX_WORD));
	cursor->to_end();
    }

    /// Destructor.
    ~HoneySpellingWordsList();

    Xapian::termcount get_approx_size() const;

    /** Returns the current termname.
     *
     *  Either next() or skip_to() must have been called before this
     *  method can be called.
     */
    std::string get_termname() const;

    /** Returns the term frequency of the current term.
     *
     *  Either next() or skip_to() must have been called before this
     *  method can be called.
     */
    Xapian::doccount get_termfreq() const;

    /// Advance to the next term in the list.
    TermList* next();

    /// Advance to the first term which is >= term.
    TermList* skip_to(const std::string& term);

    /// True if we're off the end of the list
    bool at_end() const;
};

#endif /* XAPIAN_INCLUDED_HONEY_SPELLINGWORDSLIST_H */
