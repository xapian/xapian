/** @file
 * @brief Soaktest generating lots of random queries.
 */
/* Copyright (C) 2010 Richard Boulton
 * Copyright (C) 2015 Olly Betts
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

#include <config.h>

#include "soaktest/soaktest.h"
#include "soaktest/soaktest_queries.h"

#include <xapian.h>

#include "backendmanager.h"
#include "str.h"
#include "testrunner.h"
#include "testsuite.h"
#include "testutils.h"

#include <list>

using namespace std;

/** Make a database in which docs have the fields:
 *
 *  - N: values are numbers between 1 and maxtermsperfield.  Each document
 *  contains all Nx for x from 1 to randint(maxtermsperfield).
 */
static void
builddb_queries1(Xapian::WritableDatabase &db, const string &arg)
{
    unsigned int doccount = 1000;
    unsigned int maxtermsperfield = atoi(arg.c_str());
    for (unsigned int i = 0; i < doccount; ++i) {
	Xapian::Document doc;
	for (unsigned int j = randint(maxtermsperfield) + 1; j != 0; --j) {
	    doc.add_term("N" + str(j));
	}
	db.add_document(doc);
    }
    db.commit();
}

/// The environment used by the steps when building a query.
struct QueryBuilderEnv {
    /// Workspace for the query builder steps.
    list<Xapian::Query> pieces;

    unsigned int maxtermsperfield;
    unsigned int maxchildren;

    QueryBuilderEnv(unsigned int maxtermsperfield_,
		    unsigned int maxchildren_)
	    : maxtermsperfield(maxtermsperfield_),
	      maxchildren(maxchildren_)
    {}

    /// Pop a query from the front of the list of pieces, and return it.
    Xapian::Query pop() {
	if (pieces.empty()) return Xapian::Query();
	Xapian::Query result = pieces.front();
	pieces.pop_front();
	return result;
    }

    /// Get an iterator pointing to the "num"th element in pieces.
    list<Xapian::Query>::iterator pick(unsigned int num) {
	list<Xapian::Query>::iterator i = pieces.begin();
	for (unsigned int c = 0; c != num && i != pieces.end(); ++c, ++i) {}
	return i;
    }
};

typedef void (*QueryStep)(QueryBuilderEnv &);

/// Push a leaf query on field N onto the list of query pieces.
static void push_leaf_N(QueryBuilderEnv & env)
{
    env.pieces.push_back(Xapian::Query(
	"N" + str(randint(env.maxtermsperfield) + 1)));
}

/** Combine some queries with OR.
 *
 *  The queries are removed from the list and the combined query is added to
 *  the end.
 */
static void combine_OR(QueryBuilderEnv & env)
{
    list<Xapian::Query>::iterator i = env.pick(randint(env.maxchildren));
    Xapian::Query combined(Xapian::Query::OP_OR, env.pieces.begin(), i);
    env.pieces.erase(env.pieces.begin(), i);
    env.pieces.push_back(combined);
}

/** Combine some queries with AND.
 *
 *  The queries are removed from the list and the combined query is added to
 *  the end.
 */
static void combine_AND(QueryBuilderEnv & env)
{
    list<Xapian::Query>::iterator i = env.pick(randint(env.maxchildren));
    Xapian::Query combined(Xapian::Query::OP_AND, env.pieces.begin(), i);
    env.pieces.erase(env.pieces.begin(), i);
    env.pieces.push_back(combined);
}

/** Combine some queries with XOR.
 *
 *  The queries are removed from the list and the combined query is added to
 *  the end.
 */
static void combine_XOR(QueryBuilderEnv & env)
{
    list<Xapian::Query>::iterator i = env.pick(randint(env.maxchildren));
    Xapian::Query combined(Xapian::Query::OP_XOR, env.pieces.begin(), i);
    env.pieces.erase(env.pieces.begin(), i);
    env.pieces.push_back(combined);
}

/** Combine some queries with AND_NOT.
 *
 *  The queries are removed from the list and the combined query is added to
 *  the end.
 */
static void combine_NOT(QueryBuilderEnv & env)
{
    if (env.pieces.size() < 2) return;
    list<Xapian::Query>::iterator i = env.pick(2);
    Xapian::Query combined(Xapian::Query::OP_AND_NOT, env.pieces.begin(), i);
    env.pieces.erase(env.pieces.begin(), i);
    env.pieces.push_back(combined);
}

/// Random query builder.
class QueryBuilder {
    /// The possible steps.
    vector<QueryStep> options;

    /// The environment for the build steps.
    QueryBuilderEnv env;

    /// Maximum number of steps to take when building a query.
    unsigned int maxsteps;

  public:
    QueryBuilder(unsigned int maxtermsperfield_,
		 unsigned int maxchildren_,
		 unsigned int maxsteps_)
	    : env(maxtermsperfield_, maxchildren_),
	      maxsteps(maxsteps_)
    {
	// Build up the set of options.
	// Some options are added multiple times to make them more likely.
	options.push_back(push_leaf_N);
	options.push_back(push_leaf_N);
	options.push_back(push_leaf_N);
	options.push_back(push_leaf_N);
	options.push_back(combine_OR);
	options.push_back(combine_AND);
	options.push_back(combine_XOR);
	options.push_back(combine_NOT);
    }

    /** Build a random query.
     *
     *  This performs a random number of steps, each of which modifies the
     *  QueryBuilderEnv by picking a random one of the options.
     *
     *  After the steps have been performed, the first item on the list in
     *  QueryBuilderEnv is popped and returned.
     */
    Xapian::Query make_query() {
	unsigned int steps = randint(maxsteps) + 1;
	while (steps-- != 0) {
	    QueryStep & step = options[randint(options.size())];
	    step(env);
	}
	return env.pop();
    }
};

// Generate a load of random queries, and run them checking that the first
// returned result is the same when asking for 1 result as it is when asking
// for all results.  This is a basic test that early-termination query
// optimisations aren't causing different results to be returned.
DEFINE_TESTCASE(queries1, writable && !remote && !inmemory) {
    unsigned int seed = initrand();
    unsigned int maxtermsperfield = 100;
    unsigned int repetitions = 10000;
    QueryBuilder builder(maxtermsperfield, 10, 10);

    Xapian::Database db;
    string arg(str(maxtermsperfield));
    db = backendmanager->get_database("queries1_" + str(seed) + "_" + arg,
				      builddb_queries1, arg);

    // Reset the random seed, to make results repeatable whether database was
    // created or not.
    initrand();

    Xapian::Enquire enquire(db);

    unsigned int count = 0;
    while (++count != repetitions) {
	Xapian::Query query(builder.make_query());
	tout.str(string());
	tout << "query " << count << ": " << query << "\n";

	enquire.set_query(query);
	Xapian::MSet mset1 = enquire.get_mset(0, 1);
	Xapian::MSet mset10 = enquire.get_mset(0, 10);
	Xapian::MSet msetall = enquire.get_mset(0, db.get_doccount());
	tout << mset1 << "\n";
	tout << mset10 << "\n";
	tout << msetall << "\n";
	if (mset1.empty()) {
	    TEST(mset10.empty());
	    TEST(msetall.empty());
	    continue;
	}
	TEST(mset_range_is_same(mset1, 0, msetall, 0, mset1.size()));
	TEST(mset_range_is_same(mset10, 0, msetall, 0, mset10.size()));
    }
}
