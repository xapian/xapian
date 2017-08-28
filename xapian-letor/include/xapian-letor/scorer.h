/** @file scorer.h
 * @brief Scorer class. Measure of ranking quality.
 */
/* Copyright (C) 2016 Ayush Tomar
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

#ifndef SCORER_H
#define SCORER_H


#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "featurevector.h"
#include "letor_error.h"

#include <string>

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT Scorer : public Xapian::Internal::intrusive_base {
  public:
    /// Default constructor
    Scorer() { }

    /// Virtual destructor since we have virtual methods.
    virtual ~Scorer() { }

    /// Method to return ranking score
    virtual double score(const std::vector<FeatureVector> & fvv) const = 0;

  private:
    /// Don't allow assignment.
    void operator=(const Scorer &);

    /// Don't allow copying.
    Scorer(const Scorer & o);

};

/// NDCGScore class
class XAPIAN_VISIBILITY_DEFAULT NDCGScore: public Scorer {
  public:
    ///Default constructor
    NDCGScore();

    /// Destructor
    ~NDCGScore();

    double score(const std::vector<FeatureVector> & fvv) const;

};

/** Expected Reciprocal Rank(ERR) Scorer.
 *  ERR Scorer is adapted from the paper:
 *  http://olivier.chapelle.cc/pub/err.pdf
 *  Chapelle, Metzler, Zhang, Grinspan (2009)
 *  Expected Reciprocal Rank for Graded Relevance
 */
class XAPIAN_VISIBILITY_DEFAULT ERRScore : public Scorer {
    /* max_grade is the maximum grade that a label can be mapped to.
     * The relevance label that user sets for the documents can be arbitary
     * and huge. Computing the probability of relevance for the documents,
     * which involves computing pow(2, label), can be time consuming.
     * To reduce the time complexity the labels are mapped to a smaller space
     * i.e. {0, 1, 2,...., max_grade}.
     * By default max_grade is 4.
     */
    int max_grade = 4;

  public:
    /// Default constructor
    ERRScore();

    /// Function to set max_grade
    void set_max_grade(int max_grade_);

    /// Function that returns the value set for max_grade.
    int get_max_grade();

    /// Destructor
    ~ERRScore();

    double score(const std::vector<FeatureVector> & fvv) const;

};

}

#endif /* SCORER_H */
