/** @file
 * @brief Xapian::Query API class
 */
/* Copyright (C) 2011,2012,2013,2015,2016,2017,2018 Olly Betts
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

#include <config.h>

#include "xapian/query.h"
#include "queryinternal.h"

#include <algorithm>

#include "debuglog.h"
#include "omassert.h"
#include "vectortermlist.h"

#include "xapian/error.h"

using namespace std;

namespace Xapian {

// Extra () are needed to resolve ambiguity with method declaration.
const Query Query::MatchAll((string()));

const Query Query::MatchNothing;

Query::Query(const string & term, Xapian::termcount wqf, Xapian::termpos pos)
    : internal(new Xapian::Internal::QueryTerm(term, wqf, pos))
{
    LOGCALL_CTOR(API, "Query", term | wqf | pos);
}

Query::Query(Xapian::PostingSource * source)
    : internal(new Xapian::Internal::QueryPostingSource(source))
{
    LOGCALL_CTOR(API, "Query", source);
}

Query::Query(double factor, const Xapian::Query & subquery)
{
    LOGCALL_CTOR(API, "Query", factor | subquery);

    if (!subquery.empty())
	internal = new Xapian::Internal::QueryScaleWeight(factor, subquery);
}

Query::Query(op op_, const Xapian::Query & subquery, double factor)
{
    LOGCALL_CTOR(API, "Query", op_ | subquery | factor);

    if (rare(op_ != OP_SCALE_WEIGHT))
	throw Xapian::InvalidArgumentError("op must be OP_SCALE_WEIGHT");
    // If the subquery is MatchNothing then generate Query() which matches
    // nothing.
    if (!subquery.internal.get()) return;
    switch (subquery.internal->get_type()) {
	case OP_VALUE_RANGE:
	case OP_VALUE_GE:
	case OP_VALUE_LE:
	    // These operators always return weight 0, so OP_SCALE_WEIGHT has
	    // no effect on them.
	    internal = subquery.internal;
	    return;
	default:
	    break;
    }
    internal = new Xapian::Internal::QueryScaleWeight(factor, subquery);
}

Query::Query(op op_, Xapian::valueno slot, const std::string & limit)
{
    LOGCALL_CTOR(API, "Query", op_ | slot | limit);

    if (op_ == OP_VALUE_GE) {
	if (limit.empty())
	    internal = new Xapian::Internal::QueryTerm();
	else
	    internal = new Xapian::Internal::QueryValueGE(slot, limit);
    } else if (usual(op_ == OP_VALUE_LE)) {
	internal = new Xapian::Internal::QueryValueLE(slot, limit);
    } else {
	throw Xapian::InvalidArgumentError("op must be OP_VALUE_LE or OP_VALUE_GE");
    }
}

Query::Query(op op_, Xapian::valueno slot,
	     const std::string & begin, const std::string & end)
{
    LOGCALL_CTOR(API, "Query", op_ | slot | begin | end);

    if (rare(op_ != OP_VALUE_RANGE))
	throw Xapian::InvalidArgumentError("op must be OP_VALUE_RANGE");
    // If begin > end then generate Query() which matches nothing.
    if (begin.empty()) {
	internal = new Xapian::Internal::QueryValueLE(slot, end);
    } else if (usual(begin <= end)) {
	internal = new Xapian::Internal::QueryValueRange(slot, begin, end);
    }
}

Query::Query(op op_,
	     const std::string & pattern,
	     Xapian::termcount max_expansion,
	     int max_type,
	     op combiner)
{
    LOGCALL_CTOR(API, "Query", op_ | pattern | max_expansion | max_type | combiner);
    if (rare(op_ != OP_WILDCARD))
	throw Xapian::InvalidArgumentError("op must be OP_WILDCARD");
    if (rare(combiner != OP_SYNONYM && combiner != OP_MAX && combiner != OP_OR))
	throw Xapian::InvalidArgumentError("combiner must be OP_SYNONYM or OP_MAX or OP_OR");
    internal = new Xapian::Internal::QueryWildcard(pattern,
						   max_expansion,
						   max_type,
						   combiner);
}

const TermIterator
Query::get_terms_begin() const
{
    if (!internal.get())
	return TermIterator();

    vector<pair<Xapian::termpos, string>> terms;
    internal->gather_terms(static_cast<void*>(&terms));
    sort(terms.begin(), terms.end());

    vector<string> v;
    const string * old_term = NULL;
    Xapian::termpos old_pos = 0;
    for (auto && i : terms) {
	// Remove duplicates (same term at the same position).
	if (old_term && old_pos == i.first && *old_term == i.second)
	    continue;

	v.push_back(i.second);
	old_pos = i.first;
	old_term = &(i.second);
    }
    return TermIterator(new VectorTermList(v.begin(), v.end()));
}

const TermIterator
Query::get_unique_terms_begin() const
{
    if (!internal.get())
	return TermIterator();

    vector<pair<Xapian::termpos, string>> terms;
    internal->gather_terms(static_cast<void*>(&terms));
    sort(terms.begin(), terms.end(), [](
		const pair<Xapian::termpos, string>& a,
		const pair<Xapian::termpos, string>& b) {
	return a.second < b.second;
    });

    vector<string> v;
    const string * old_term = NULL;
    for (auto && i : terms) {
	// Remove duplicate term names.
	if (old_term && *old_term == i.second)
	    continue;

	v.push_back(i.second);
	old_term = &(i.second);
    }
    return TermIterator(new VectorTermList(v.begin(), v.end()));
}

Xapian::termcount
Query::get_length() const XAPIAN_NOEXCEPT
{
    return (internal.get() ? internal->get_length() : 0);
}

string
Query::serialise() const
{
    string result;
    if (internal.get())
	internal->serialise(result);
    return result;
}

const Query
Query::unserialise(const string & s, const Registry & reg)
{
    const char * p = s.data();
    const char * end = p + s.size();
    Query::Internal * q = Query::Internal::unserialise(&p, end, reg);
    AssertEq(p, end);
    return Query(q);
}

Xapian::Query::op
Query::get_type() const XAPIAN_NOEXCEPT
{
    if (!internal.get())
	return Xapian::Query::LEAF_MATCH_NOTHING;
    return internal->get_type();
}

size_t
Query::get_num_subqueries() const XAPIAN_NOEXCEPT
{
    return internal.get() ? internal->get_num_subqueries() : 0;
}

const Query
Query::get_subquery(size_t n) const
{
    return internal->get_subquery(n);
}

string
Query::get_description() const
{
    string desc = "Query(";
    if (internal.get())
	desc += internal->get_description();
    desc += ")";
    return desc;
}

void
Query::init(op op_, size_t n_subqueries, Xapian::termcount parameter)
{
    if (parameter > 0 &&
	op_ != OP_NEAR && op_ != OP_PHRASE && op_ != OP_ELITE_SET)
	throw InvalidArgumentError("parameter only valid with OP_NEAR, "
				   "OP_PHRASE or OP_ELITE_SET");

    switch (op_) {
	case OP_AND:
	    internal = new Xapian::Internal::QueryAnd(n_subqueries);
	    break;
	case OP_OR:
	    internal = new Xapian::Internal::QueryOr(n_subqueries);
	    break;
	case OP_AND_NOT:
	    internal = new Xapian::Internal::QueryAndNot(n_subqueries);
	    break;
	case OP_XOR:
	    internal = new Xapian::Internal::QueryXor(n_subqueries);
	    break;
	case OP_AND_MAYBE:
	    internal = new Xapian::Internal::QueryAndMaybe(n_subqueries);
	    break;
	case OP_FILTER:
	    internal = new Xapian::Internal::QueryFilter(n_subqueries);
	    break;
	case OP_NEAR:
	    internal = new Xapian::Internal::QueryNear(n_subqueries,
						       parameter);
	    break;
	case OP_PHRASE:
	    internal = new Xapian::Internal::QueryPhrase(n_subqueries,
							 parameter);
	    break;
	case OP_ELITE_SET:
	    internal = new Xapian::Internal::QueryEliteSet(n_subqueries,
							   parameter);
	    break;
	case OP_SYNONYM:
	    internal = new Xapian::Internal::QuerySynonym(n_subqueries);
	    break;
	case OP_MAX:
	    internal = new Xapian::Internal::QueryMax(n_subqueries);
	    break;
	default:
	    if (op_ == OP_INVALID && n_subqueries == 0) {
		internal = new Xapian::Internal::QueryInvalid();
		break;
	    }
	    throw InvalidArgumentError("op not valid with a list of subqueries");
    }
}

void
Query::add_subquery(bool positional, const Xapian::Query & subquery)
{
    // We could handle this in a type-safe way, but we'd need to at least
    // declare Xapian::Internal::QueryBranch in the API header, which seems
    // less desirable than a static_cast<> here.
    Xapian::Internal::QueryBranch * branch_query =
	static_cast<Xapian::Internal::QueryBranch*>(internal.get());
    Assert(branch_query);
    if (positional) {
	switch (subquery.get_type()) {
	    case LEAF_TERM:
		break;
	    case LEAF_POSTING_SOURCE:
	    case LEAF_MATCH_ALL:
	    case LEAF_MATCH_NOTHING:
		// None of these have positions, so positional operators won't
		// match.  Add MatchNothing as that is has special handling in
		// AND-like queries to reduce the parent query to MatchNothing,
		// which is appropriate in this case.
		branch_query->add_subquery(MatchNothing);
		return;
	    case OP_OR:
		// OP_OR is now handled below OP_NEAR and OP_PHRASE.
		break;
	    default:
		throw Xapian::UnimplementedError("OP_NEAR and OP_PHRASE only currently support leaf subqueries");
	}
    }
    branch_query->add_subquery(subquery);
}

void
Query::done()
{
    Xapian::Internal::QueryBranch * branch_query =
	static_cast<Xapian::Internal::QueryBranch*>(internal.get());
    if (branch_query)
	internal = branch_query->done();
}

}
