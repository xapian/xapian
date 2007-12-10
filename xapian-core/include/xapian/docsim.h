/** \file docsim.h
 * \brief API for calculating similarities between documents
 */
/* Copyright 2007 Yung-chung Lin
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_DOCSIM_H
#define XAPIAN_INCLUDED_DOCSIM_H

#include <string>
#include <xapian/database.h>
#include <xapian/visibility.h>

namespace Xapian {

class Document;

/// Base class of document similarity calculation
class XAPIAN_VISIBILITY_DEFAULT DocSim {
    
  protected:
    /// The database from which documents are retrieved.
    Database db;
    
  public:
    /// Make a new empty DocSim
    DocSim() { }

    /// Destructor
    virtual ~DocSim();
    
    /// Specify database
    void set_database(const Database & db_) { db = db_; }
    
    /// Calculate the similarity between two documents.
    virtual double calculate_similarity(const Document & a,
                                        const Document & b) const = 0;
    
    /// Return a string describing this object.
    virtual std::string get_description() const = 0;
};
    
/// Cosine similarity
class XAPIAN_VISIBILITY_DEFAULT DocSimCosine : public DocSim {
  public:
    /// Make a new empty DocSim
    DocSimCosine() { }
    
    /// Destructor
    ~DocSimCosine();
    
    /// Calculate the similarity between two documents.
    double calculate_similarity(const Document & a, const Document & b) const;
    
    /** Introspection method.
     *
     *  @return  A string representing this DocSim.
     */
    std::string get_description() const;
};
    
}

#endif // XAPIAN_INCLUDED_DOCSIM_H
