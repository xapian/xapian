// Simple test that we can load the xapian module and run a simple test
//
// Copyright (C) 2004,2005,2006,2007,2008,2011 Olly Betts
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

// The Portable.NET compiler has a bug which prevents it finding static
// member functions such as Xapian.Version.Major():
//
// http://savannah.gnu.org/bugs/?func=detailitem&item_id=12231
//
// The bug is fixed in Portable.NET CVS HEAD - the fix should make it
// into Portable.NET 0.8.2.
//
// The workaround for now is to add an explicit "using Xapian;" here:
using Xapian;

class TestMatchDecider : Xapian.MatchDecider {
    public override bool Apply(Xapian.Document doc) {
	return (doc.GetValue(0) == "yes");
    }
}

class SmokeTest {
    public static void Main() {
	try {
	    // Test the version number reporting functions give plausible
	    // results.
	    string v = "";
	    v += Xapian.Version.Major();
	    v += ".";
	    v += Xapian.Version.Minor();
	    v += ".";
	    v += Xapian.Version.Revision();
	    string v2 = Xapian.Version.String();
	    if (v != v2) {
		System.Console.WriteLine("Unexpected version output (" + v + " != " + v2 + ")");
		System.Environment.Exit(1);
	    }

	    Xapian.Stem stem = new Xapian.Stem("english");
	    Xapian.Document doc = new Xapian.Document();
	    // Currently SWIG doesn't generate zero-byte clean code for
	    // transferring strings between C# and C++.
	    /*
	    doc.SetData("a\0b");
	    if (doc.GetData() == "a") {
		System.Console.WriteLine("GetData+SetData truncates at a zero byte");
		System.Environment.Exit(1);
	    }
	    if (doc.GetData() != "a\0b") {
		System.Console.WriteLine("GetData+SetData doesn't transparently handle a zero byte");
		System.Environment.Exit(1);
	    }
	    */
	    doc.SetData("is there anybody out there?");
	    doc.AddTerm("XYzzy");
	    doc.AddPosting(stem.Apply("is"), 1);
	    doc.AddPosting(stem.Apply("there"), 2);
	    doc.AddPosting(stem.Apply("anybody"), 3);
	    doc.AddPosting(stem.Apply("out"), 4);
	    doc.AddPosting(stem.Apply("there"), 5);

	    Xapian.WritableDatabase db = Xapian.InMemory.Open();
	    db.AddDocument(doc);
	    if (db.GetDocCount() != 1) {
		System.Environment.Exit(1);
	    }

	    if (doc.TermListCount() != 5) {
		System.Environment.Exit(1);
	    }
	    int count = 0;
	    Xapian.TermIterator i = doc.TermListBegin();
	    while (i != doc.TermListEnd()) {
		++count;
		++i;
	    }
	    if (count != 5) {
		System.Environment.Exit(1);
	    }

	    // Check exception handling for Xapian::DocNotFoundError.
	    try {
		Xapian.Document doc2 = db.GetDocument(2);
		System.Console.WriteLine("Retrieved non-existent document: " + doc2.ToString());
		System.Environment.Exit(1);
	    } catch (System.Exception e) {
		// We expect DocNotFoundError
		if (e.Message.Substring(0, 16) != "DocNotFoundError") {
		    System.Console.WriteLine("Unexpected exception from accessing non-existent document: " + e.Message);
		    System.Environment.Exit(1);
		}
	    }

	    // Check QueryParser parsing error.
	    try {
		Xapian.QueryParser qp = new Xapian.QueryParser();
		qp.ParseQuery("test AND");
		System.Console.WriteLine("Successfully parsed bad query");
		System.Environment.Exit(1);
	    } catch (System.Exception e) {
		if (e.Message != "QueryParserError: Syntax: <expression> AND <expression>") {
		    System.Console.WriteLine("Exception string not as expected, got: '" + e.Message + "'");
		    System.Environment.Exit(1);
		}
	    }

	    {
		Xapian.QueryParser qp = new Xapian.QueryParser();
		// FIXME: It would be better if the (uint) cast wasn't required
		// here.
		qp.ParseQuery("hello world", (uint)Xapian.QueryParser.feature_flag.FLAG_BOOLEAN);
	    }

            if (Xapian.Query.MatchAll.GetDescription() != "Xapian::Query(<alldocuments>)") {
		System.Console.WriteLine("Unexpected Query.MatchAll.toString()");
		System.Environment.Exit(1);
            }

            if (Xapian.Query.MatchNothing.GetDescription() != "Xapian::Query()") {
		System.Console.WriteLine("Unexpected Query.MatchNothing.toString()");
		System.Environment.Exit(1);
            }

	    // Check that OP_ELITE_SET works (in 0.9.6 and earlier it had the
	    // wrong value in C#).
	    try {
		Xapian.Query foo = new Xapian.Query(Xapian.Query.op.OP_OR, "hello", "world");
		Xapian.Query foo2 = new Xapian.Query(Xapian.Query.op.OP_ELITE_SET, foo, foo);
		foo = foo2; // Avoid "unused variable" warning.
	    } catch (System.Exception e) {
		System.Console.WriteLine("Using OP_ELITE_SET cause exception '" + e.Message + "'");
		System.Environment.Exit(1);
	    }

	    // Feature test for MatchDecider.
	    doc = new Xapian.Document();
	    doc.SetData("Two");
	    doc.AddPosting(stem.Apply("out"), 1);
	    doc.AddPosting(stem.Apply("source"), 2);
	    doc.AddValue(0, "yes");
	    db.AddDocument(doc);

	    Xapian.Query query = new Xapian.Query(stem.Apply("out"));
	    Xapian.Enquire enquire = new Xapian.Enquire(db);
	    enquire.SetQuery(query);
	    Xapian.MSet mset = enquire.GetMSet(0, 10, null, new TestMatchDecider());
	    if (mset.Size() != 1) {
		System.Console.WriteLine("MatchDecider found " + mset.Size().ToString() + " documents, expected 1");
		System.Environment.Exit(1);
	    }
	    if (mset.GetDocId(0) != 2) {
		System.Console.WriteLine("MatchDecider mset has wrong docid in");
		System.Environment.Exit(1);
	    }

	    mset = enquire.GetMSet(0, 10);
	    for (Xapian.MSetIterator m = mset.Begin(); m != mset.End(); m++) {
		// In Xapian 1.2.6 and earlier, the iterator would become
		// eligible for garbage collection after being advanced.
		// It didn't actually get garbage collected often, but when
		// it did, it caused a crash.  Here we force a GC run to make
		// this issue manifest if it is present.
		System.GC.Collect();
		System.GC.WaitForPendingFinalizers();
	    }

            // Test setting and getting metadata
            if (db.GetMetadata("Foo") !=  "") {
		System.Console.WriteLine("db.GetMetadata(\"Foo\") returned wrong value \"" + db.GetMetadata("Foo") + "\" - expected \"\"");
		System.Environment.Exit(1);
            }
            db.SetMetadata("Foo", "Foo");
            if (db.GetMetadata("Foo") !=  "Foo") {
		System.Console.WriteLine("db.GetMetadata(\"Foo\") returned wrong value \"" + db.GetMetadata("Foo") + "\" - expected \"Foo\"");
		System.Environment.Exit(1);
            }

	    // Test OP_SCALE_WEIGHT and corresponding constructor
	    Xapian.Query query4 = new Xapian.Query(Xapian.Query.op.OP_SCALE_WEIGHT, new Xapian.Query("foo"), 5.0);
	    if (query4.GetDescription() != "Xapian::Query(5 * foo)") {
		System.Console.WriteLine("Unexpected query4.GetDescription()");
		System.Environment.Exit(1);
	    }

	} catch (System.Exception e) {
	    System.Console.WriteLine("Exception: " + e.ToString());
	    System.Environment.Exit(1);
	}
    }
}
