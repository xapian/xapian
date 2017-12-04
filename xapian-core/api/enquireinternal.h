/** @file enquireinternal.h
 * @brief Xapian::Enquire internals
 */
/* Copyright 2017 Olly Betts
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

#ifndef XAPIAN_INCLUDED_ENQUIREINTERNAL_H
#define XAPIAN_INCLUDED_ENQUIREINTERNAL_H

#include "backends/databaseinternal.h"
#include "xapian/constants.h"
#include "xapian/database.h"
#include "xapian/enquire.h"
#include "xapian/intrusive_ptr.h"
#include "xapian/keymaker.h"
#include "xapian/matchspy.h"
#include "xapian/mset.h" // Only needed to forward declare MSet::Internal.
#include "xapian/query.h"

#include <memory>
#include <string>
#include <vector>

namespace Xapian {

class ESet;
class RSet;
class Weight;

class Enquire::Internal : public Xapian::Internal::intrusive_base {
    friend class Enquire;
    friend class MSet::Internal;

  public:
    typedef enum { REL, VAL, VAL_REL, REL_VAL, DOCID } sort_setting;

  private:
    Xapian::Database db;

    Xapian::Query query;

    mutable Xapian::termcount query_length = 0;

    mutable std::unique_ptr<Xapian::Weight> weight;

    docid_order order = Xapian::Enquire::ASCENDING;

    sort_setting sort_by = REL;

    Xapian::Internal::opt_intrusive_ptr<Xapian::KeyMaker> sort_functor;

    Xapian::valueno sort_key = Xapian::BAD_VALUENO;

    bool sort_val_reverse = false;

    Xapian::valueno collapse_key = Xapian::BAD_VALUENO;

    Xapian::doccount collapse_max = 0;

    int percent_threshold = 0;

    double weight_threshold = 0.0;

    std::vector<Xapian::Internal::opt_intrusive_ptr<MatchSpy>> matchspies;

    double time_limit = 0.0;

    enum { EXPAND_TRAD, EXPAND_BO1 } eweight = EXPAND_TRAD;

    double expand_k = 1.0;

  public:
    explicit
    Internal(const Database& db_);

    MSet get_mset(doccount first,
		  doccount maxitems,
		  doccount checkatleast,
		  const RSet* rset,
		  const MatchDecider* mdecider) const;

    TermIterator get_matching_terms_begin(docid did) const;

    ESet get_eset(termcount maxitems,
		  const RSet& rset,
		  int flags,
		  const ExpandDecider* edecider_,
		  double min_weight) const;

    doccount get_termfreq(const std::string& term) const {
	return db.get_termfreq(term);
    }

    Document get_document(docid did) const {
	// This is called by MSetIterator, so we know the document exists.
	return db.get_document(did, Xapian::DOC_ASSUME_VALID);
    }

    void request_document(docid did) const {
	db.internal->request_document(did);
    }
};

}

#endif // XAPIAN_INCLUDED_ENQUIREINTERNAL_H
