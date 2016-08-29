/** @file points.h
 *  @brief Classes for FreqSource
 */
/* Copyright (C) 2016 Richhiey Thomas
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef POINTS_H
#define POINTS_H

#include <xapian.h>

#include <unordered_map>

namespace Xapian {

/// Abstract class used for construction of the TermListGroup
class FreqSource {

  public:

    /** This contains a map of the terms and its corresponding term frequencies.
     *  The term frequency of a term stands for the number of documents it indexes
     */
    std::unordered_map<std::string, doccount> termfreq;

    /// Destructor
    virtual ~FreqSource();

    /// This method returns the term frequency of a particular term 'tname'
    virtual doccount get_termfreq(const std::string &tname) = 0;

    /// This method returns the number of documents
    virtual doccount get_doccount() const = 0;
};

/// A class for dummy frequency source for construction of termlists
class DummyFreqSource : public FreqSource {

  public:

    /// This method returns the value 1 as a dummy term frequency
    doccount get_termfreq(const std::string &);

    /// This method returns the total number of documents
    doccount get_doccount() const;
};

/// A class for construction of termlists
class TermListGroup : public FreqSource {

  public:

    /** This method adds document terms and their statistics to the FreqSource
     *  for construction of termlists
     */
    doccount docs_num;

    /// This method adds a single document and calculates its corresponding stats
    void add_document(const Document &doc);

    /// This method adds a number of documents from the DocumentSource
    void add_documents(const MSet &docs);

    /** This method returns the number of documents that the term 'tname' exists in
     *  or the number of documents that a certain term indexes
     */
    doccount get_termfreq(const std::string &tname);

    /// This method returns the total number of documents
    doccount get_doccount() const;
};
}

#endif
