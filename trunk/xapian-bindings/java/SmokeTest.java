// Simple test that we can use xapian from java
//
// Copyright (C) 2005,2006,2007,2008,2011 Olly Betts
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

import org.xapian.*;

// FIXME: need to sort out throwing wrapped Xapian::Error subclasses
//import org.xapian.errors.*;

// FIXME: "implements" not "extends" in JNI Java API
class MyMatchDecider extends MatchDecider {
    public boolean accept(Document d) {
	// NB It's not normally appropriate to call getData() in a MatchDecider
	// but we do it here to make sure we don't get an empty document.
/*	try { */
	    return d.getData().length() == 0;
/*
	} catch (XapianError e) {
	    return true;
	}
*/
    }
}

// FIXME: "implements" not "extends" in JNI Java API
class MyExpandDecider extends ExpandDecider {
    public boolean accept(String s) { return s.charAt(0) != 'a'; }
}

public class SmokeTest {
    public static void main(String[] args) throws Exception {
	try {
	    Stem stem = new Stem("english");
	    if (!stem.toString().equals("Xapian::Stem(english)")) {
		System.err.println("Unexpected stem.toString()");
		System.exit(1);
	    }
	    Document doc = new Document();
	    doc.setData("a\000b");
	    String s = doc.getData();
	    if (s.equals("a")) {
		System.err.println("getData+setData truncates at a zero byte");
		System.exit(1);
	    }
	    if (!s.equals("a\000b")) {
		System.err.println("getData+setData doesn't transparently handle a zero byte");
		System.exit(1);
	    }
	    doc.setData("is there anybody out there?");
	    doc.addTerm("XYzzy");
// apply was stemWord() in the JNI bindings
	    doc.addPosting(stem.apply("is"), 1);
	    doc.addPosting(stem.apply("there"), 2);
	    doc.addPosting(stem.apply("anybody"), 3);
	    doc.addPosting(stem.apply("out"), 4);
	    doc.addPosting(stem.apply("there"), 5);
	    // FIXME: was WritableDatabase db = Xapian.InMemory.open();
	    WritableDatabase db = InMemory.open();
	    db.addDocument(doc);
	    if (db.getDocCount() != 1) {
		System.err.println("Unexpected db.getDocCount()");
		System.exit(1);
	    }

            if (!Query.MatchAll.toString().equals("Query(<alldocuments>)")) {
		System.err.println("Unexpected Query.MatchAll.toString()");
		System.exit(1);
            }

            if (!Query.MatchNothing.toString().equals("Query()")) {
		System.err.println("Unexpected Query.MatchNothing.toString()");
		System.exit(1);
            }

	    String[] terms = { "smoke", "test", "terms" };
	    Query query = new Query(Query.OP_OR, terms);
	    if (!query.toString().equals("Query((smoke OR test OR terms))")) {
		System.err.println("Unexpected query.toString()");
		System.exit(1);
	    }
	    Query[] queries = { new Query("smoke"), query, new Query("string") };
	    Query query2 = new Query(Query.OP_XOR, queries);
	    if (!query2.toString().equals("Query((smoke XOR (smoke OR test OR terms) XOR string))")) {
		System.err.println("Unexpected query2.toString()");
		System.exit(1);
	    }
	    String[] subqs = { "a", "b" };
	    Query query3 = new Query(Query.OP_OR, subqs);
	    if (!query3.toString().equals("Query((a OR b))")) {
		System.err.println("Unexpected query3.toString()");
		System.exit(1);
	    }
	    Enquire enq = new Enquire(db);
	    enq.setQuery(new Query(Query.OP_OR, "there", "is"));
	    MSet mset = enq.getMSet(0, 10);
	    if (mset.size() != 1) {
		System.err.println("Unexpected mset.size()");
		System.exit(1);
	    }
	    MSetIterator m_itor = mset.begin();
	    Document m_doc = null;
	    long m_id;
	    while(m_itor.hasNext()) {
		m_id = m_itor.next();
		if(m_itor.hasNext()) {
		    m_doc = mset.getDocument(m_id);
		}
	    }

	    // Only one doc exists in this mset
	    if(m_doc != null && m_doc.getDocId() != 0) {
		System.err.println("Unexpected docid");
		    System.exit(1);
	    }

	    String term_str = "";
	    TermIterator itor = enq.getMatchingTermsBegin(mset.getElement(0));
	    while (itor.hasNext()) {
		term_str += itor.next();
		if (itor.hasNext())
		    term_str += ' ';
	    }
	    if (!term_str.equals("is there")) {
		System.err.println("Unexpected term_str");
		System.exit(1);
	    }
/* FIXME:dc: Fails since Xapian::Error is still unmapped
	    boolean ok = false;
	    try {
		Database db_fail = new Database("NOsuChdaTabASe");
		// Ignore the return value.
		db_fail.getDocCount();
	    } catch (DatabaseOpeningError e) {
		ok = true;
	    }
	    if (!ok) {
		System.err.println("Managed to open non-existent database");
		System.exit(1);
	    }
*/
/*
	    if (Query.OP_ELITE_SET != 10) {
		System.err.println("OP_ELITE_SET is " + Query.OP_ELITE_SET + " not 10");
		System.exit(1);
	    }
*/
	    RSet rset = new RSet();
	    rset.addDocument(1);
	    ESet eset = enq.getESet(10, rset, new MyExpandDecider());
	    // FIXME: temporary simple check
	    if (0 == eset.size()) {
		System.err.println("ESet.size() was 0");
		System.exit(1);
	    }

	    int count = 0;
	    for(ESetIterator eit = eset.begin(); eit.hasNext(); ) {
	    // for (int i = 0; i < eset.size(); i++) {
		if (eit.getTerm().charAt(0) == 'a') {
		    System.err.println("MyExpandDecider wasn't used");
		    System.exit(1);
		}
		++count;
		eit.next();
	    }
	    if (count != eset.size()) {
		System.err.println("ESet.size() mismatched number of terms returned by ESetIterator");
		System.err.println(count + " " + eset.size());
		System.exit(1);
	    }

/*
	    MSet mset2 = enq.getMSet(0, 10, null, new MyMatchDecider());
	    if (mset2.size() > 0) {
		System.err.println("MyMatchDecider wasn't used");
		System.exit(1);
	    }
*/

	    if (!enq.getQuery().toString().equals("Query((there OR is))")) {
		System.err.println("Enquire::getQuery() returned the wrong query: " + enq.getQuery().toString());
		System.exit(1);
	    }
	} catch (Exception e) {
	    System.err.println("Caught unexpected exception " + e.toString());
	    System.exit(1);
	}
    }
}
