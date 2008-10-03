/** @file exprpostingsource.h
 * @brief Expression evaluator source of posting information
 */
/* Copyright (C) 2008 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#ifndef XAPIAN_INCLUDED_EXPRPOSTINGSOURCE_H
#define XAPIAN_INCLUDED_EXPRPOSTINGSOURCE_H

#include <xapian/postingsource.h>

#include <string>
#include <map>

class Parser;

namespace Xapian {

/** A posting source which evaluates an expression whose domain is document values.
 *
 *
 *  This returns entries for all documents in the given database which have
 *  non-empty values in slots referred to by the specified expression. It
 *  returns a weight calculated by applying sortable_unserialise to the
 *  values stored in the referenced slots (so the values stored should
 *  probably have been calculated by applying sortable_serialise to a
 *  floating point number at index time), and evaluating the expression
 *  based on the unserialised values.
 *
 *  The weight returned is calculated from an expression supplied to the 
 *  constructor, e.g.:
 *
 *      (InDegree * 2.0) + (OutDegree * 1.5)
 *
 *  where InDegree and OutDegree are mapped to values in the document
 *  according to the property map supplied to the constructor. The property
 *  map is a map from property name to document value number. Behaviour
 *  is unspecified when a property in the expression is not present in the
 *  property map.
 *
 *  A defaults map is also supplied to the constructor. This may or may not
 *  contain a default value for each property. If a value referred to by
 *  the expression is empty, the default value is used; if no default is
 *  given then the document is skipped.
 *
 *  The constructor specifies a maximum weight. Any weights calculated higher than
 *  this will be truncated to the specified max. Also any negative weights will be
 *  normalised to zero.
 */
class XAPIAN_VISIBILITY_DEFAULT ExprWeightPostingSource : public PostingSource {

  public:

    typedef std::map<std::string, Xapian::valueno> PropertyMap;

    typedef std::map<std::string, double> DefaultMap;

  private:

    /// The Xapian db this posting source is attached to
    Xapian::Database db;

    /// The current document ID (0 to indicate that we haven't started yet).
    Xapian::docid current_docid;

    /// The last document ID available.
    Xapian::docid last_docid;

    /// An upper bound on the term frequency.
    Xapian::doccount termfreq_max;
    
    /// The expression used to evaluate weight
    std::string expression;
    
    /// The maximum weight to be returned
    Xapian::weight max_weight;

    /// The evaluated weight for the current doc
    double current_weight;
    
    /// Map from property name to value number
    PropertyMap prop_map;

    /// Map from property name to default value
    DefaultMap default_map;

    // Load the parser with values of the required properties from the document.
    // Returns false if a value with no default was missing, otherwise true.
    bool get_property_values(const Xapian::Document& doc, Parser& parser);
    
  public:

    /** Construct an ExprWeightPostingSource.
     *
     *  @param prop_map_ The property name to value number map.
     *  @param default_map_ The property name to default value map.
     *  @param expression_ The expression used to evaluate weights.
     *  @param max_weight_ An upper bound for evaluated weights.
     */
    ExprWeightPostingSource(PropertyMap prop_map_,
                            DefaultMap default_map_,
                            Xapian::weight max_weight_);

    void set_expression(std::string expression_);

    Xapian::doccount get_termfreq_min() const;
    Xapian::doccount get_termfreq_est() const;
    Xapian::doccount get_termfreq_max() const;

    Xapian::weight get_maxweight() const;
    Xapian::weight get_weight() const;

    void next(Xapian::weight min_wt);
    void skip_to(Xapian::docid min_docid, Xapian::weight min_wt);

    bool at_end() const;

    Xapian::docid get_docid() const;

    ExprWeightPostingSource * clone() const;
    std::string name() const;
    std::string serialise() const;
    PostingSource * unserialise(const std::string &s) const;

    void reset(const Database & db_);

    std::string get_description() const;
};

}

#endif // XAPIAN_INCLUDED_EXPRPOSTINGSOURCE_H
