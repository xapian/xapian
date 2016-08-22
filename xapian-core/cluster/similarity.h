/** @file similarity.h
 *  @brief Document Similarity Calculation API
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

#ifndef SIMILARITY_H
#define SIMILARITY_H

#include <xapian/types.h>

namespace Xapian {

/// Base class for calculating the similarity between documents
class Similarity {

  public:

    /// Destructor
    virtual ~Similarity();

    /// Calculates the similarity between the two documents
    virtual double similarity(PointType &a, PointType &b) = 0;

    /// Returns description of the similarity metric being used
    virtual std::string get_description() = 0;
};

/// Class for calculating Euclidian Distance between two points
class EuclidianDistance : public Similarity {

  public:

    /** This method calculates and returns the euclidian distance using the
     *  Euclidian distance formula
     */
    double similarity(PointType &a, PointType &b);

    /// This method returns the description of Euclidian Distance
    std::string get_description();
};

// Class for calculating the cosine distance between two documents
class CosineDistance : public Similarity {

  public:

    /** This method calculates and returns the cosine similarity using the
     *  formula  cos(theta) = a.b/(|a|*|b|)
     */
    double similarity(PointType &a, PointType &b);

    /// This method returns the description of Cosine Similarity
    std::string get_description();
};
};

#endif
