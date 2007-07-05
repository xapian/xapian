// Simple example program demonstrating query expansion.
//
// Copyright (c) 2003 James Aylett
// Copyright (c) 2004,2006,2007 Olly Betts
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
// USA

using System;

class SimpleExpand {
    public static void Main(string[] argv) {
	// We require at least two command line arguments.
	if (argv.Length < 2) {
	    Console.Error.WriteLine("Usage: SimpleExpand PATH_TO_DATABASE QUERY [-- [DOCID...]]");
	    Environment.Exit(1);
	}

	try {
	    // Open the database for searching.
	    Xapian.Database database = new Xapian.Database(argv[0]);

	    // Start an enquire session.
	    Xapian.Enquire enquire = new Xapian.Enquire(database);

	    // Create an RSet to add the listed docids to.
	    Xapian.RSet rset = new Xapian.RSet();

	    // Combine command line arguments up to "--" with spaces between
	    // them, so that simple queries don't have to be quoted at the
	    // shell level.
	    string query_string = argv[1];
	    for (int i = 2; i < argv.Length; ++i) {
		if (argv[i] == "--") {
		    while (++i < argv.Length) {
			rset.AddDocument(Convert.ToUInt32(argv[i]));
		    }
		    break;
		}
		query_string += ' ';
		query_string += argv[i];
	    }

	    // Parse the query string to produce a Xapian::Query object.
	    Xapian.QueryParser qp = new Xapian.QueryParser();
	    Xapian.Stem stemmer = new Xapian.Stem("english");
	    qp.SetStemmer(stemmer);
	    qp.SetDatabase(database);
	    qp.SetStemmingStrategy(Xapian.QueryParser.stem_strategy.STEM_SOME);
	    Xapian.Query query = qp.ParseQuery(query_string);
	    Console.WriteLine("Parsed query is: " + query.GetDescription());

	    // Find the top 10 results for the query.
	    enquire.SetQuery(query);
	    Xapian.MSet matches = enquire.GetMSet(0, 10, rset);

	    // Display the results.
	    Console.WriteLine("{0} results found.", matches.GetMatchesEstimated());
	    Console.WriteLine("Matches 1-{0}:", matches.Size());

	    Xapian.MSetIterator m = matches.Begin();
	    while (m != matches.End()) {
		Console.WriteLine("{0}: {1}% docid={2} [{3}]\n",
				  m.GetRank() + 1,
				  m.GetPercent(),
				  m.GetDocId(),
				  m.GetDocument().GetData());
		++m;
	    }

	    // If no relevant docids were given, invent an RSet containing the
	    // top 5 matches (or all the matches if there are less than 5).
	    if (rset.Empty()) {
		int c = 5;
		Xapian.MSetIterator i = matches.Begin();
		while (c-- > 0 && i != matches.End()) {
		    rset.AddDocument(i.GetDocId());
		    ++i;
		}
	    }

	    // Generate an ESet containing terms that the user might want to
	    // add to the query.
	    Xapian.ESet eset = enquire.GetESet(10, rset);

	    Console.WriteLine(eset.Size());
	    // List the terms.
	    for (Xapian.ESetIterator t = eset.Begin(); t != eset.End(); ++t) {
		Console.WriteLine("{0}: weight = {1}",
				  t.GetTerm(), t.GetWeight());
	    }
	} catch (Exception e) {
	    Console.Error.WriteLine("Exception: " + e.ToString());
	    Environment.Exit(1);
	}
    }
}
