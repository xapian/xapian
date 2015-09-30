/* api_encryption.cc: tests with encrypted databases
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2011,2012,2013,2015 Olly Betts
 * Copyright 2006,2007,2008,2009 Lemur Consulting Ltd
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

#include "api_encryption.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include "safesysstat.h" // For mkdir().
#include "safeunistd.h" // For sleep().

#include <xapian.h>

#include "backendmanager.h"
#include "backendmanager_local.h"
#include "testsuite.h"
#include "testutils.h"
#include "unixcmds.h"

#include "apitest.h"

using namespace std;

// #######################################################################
// # Tests start here

const std::string test_encryption_key = "pQHQNZrTghIryAukoxkavLQupfDqZTKnKqFceofiKlNJaCkBsCynRWxQnJlVjGBF";
const std::string test_encryption_cipher = "Serpent/XTS";

DEFINE_TESTCASE(encrypted_db_chert, chert && !remote && writable) {
  SKIP_TEST_FOR_BACKEND("inmemory");

  Xapian::WritableDatabase db(".chert_encrypted",
      Xapian::DB_CREATE_OR_OVERWRITE|Xapian::DB_BACKEND_CHERT, 8192,
      &test_encryption_key, test_encryption_cipher);

  // Read-only tests:
  {
    TEST_EQUAL(db.get_doccount(), 0);
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("word"));
    Xapian::MSet mset = enquire.get_mset(0, 10);
    TEST(mset.empty());
  }
  {
    TEST_EQUAL(db.get_doccount(), 0);
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("word"));
    Xapian::MSet mset = enquire.get_mset(0, 10);
    TEST(mset.empty());
  }

  // Writable tests:
  {
    Xapian::WritableDatabase database(db);
    Xapian::docid did;
    Xapian::Document document_in;
    document_in.set_data("Foobar rising");
    document_in.add_value(7, "Value7");
    document_in.add_value(13, "Value13");
    document_in.add_posting("foo", 1);
    document_in.add_posting("bar", 2);

    {
      did = database.add_document(document_in);
      TEST_EQUAL(did, 1);
      TEST_EQUAL(database.get_doccount(), 1);
      TEST_EQUAL(database.get_avlength(), 2);
    }

    {
      document_in.remove_term("foo");
      document_in.add_posting("baz", 1);

      database.replace_document(1, document_in);

      database.delete_document(1);

      TEST_EQUAL(database.get_doccount(), 0);
      TEST_EQUAL(database.get_avlength(), 0);
      TEST_EQUAL(database.get_termfreq("foo"), 0);
      TEST_EQUAL(database.get_collection_freq("foo"), 0);
      TEST_EQUAL(database.get_termfreq("bar"), 0);
      TEST_EQUAL(database.get_collection_freq("bar"), 0);
      TEST_EQUAL(database.get_termfreq("baz"), 0);
      TEST_EQUAL(database.get_collection_freq("baz"), 0);
    }
  }

  {
    TEST_EQUAL(db.get_doccount(), 0);
    db.add_document(Xapian::Document());
    TEST_EQUAL(db.get_doccount(), 1);
    db.add_document(Xapian::Document());
    TEST_EQUAL(db.get_doccount(), 2);
  }

  {
    for (Xapian::doccount i = 0; i < 2100; ++i) {
      Xapian::Document doc;
      for (Xapian::termcount t = 0; t < 100; ++t) {
        string term("foo");
        term += char(t ^ 70 ^ i);
        doc.add_posting(term, t);
      }
      db.add_document(doc);
    }
    TEST(db.term_exists("foo1"));
  }
  return true;
}

DEFINE_TESTCASE(encrypted_db_glass, glass && !remote && writable) {
  SKIP_TEST_FOR_BACKEND("inmemory");

  Xapian::WritableDatabase db(".glass_encrypted",
      Xapian::DB_CREATE_OR_OVERWRITE|Xapian::DB_BACKEND_GLASS, 8192,
      &test_encryption_key, test_encryption_cipher);

  // Read-only tests:
  {
    TEST_EQUAL(db.get_doccount(), 0);
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("word"));
    Xapian::MSet mset = enquire.get_mset(0, 10);
    TEST(mset.empty());
  }
  {
    TEST_EQUAL(db.get_doccount(), 0);
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("word"));
    Xapian::MSet mset = enquire.get_mset(0, 10);
    TEST(mset.empty());
  }

  // Writable tests:
  {
    Xapian::WritableDatabase database(db);
    Xapian::docid did;
    Xapian::Document document_in;
    document_in.set_data("Foobar rising");
    document_in.add_value(7, "Value7");
    document_in.add_value(13, "Value13");
    document_in.add_posting("foo", 1);
    document_in.add_posting("bar", 2);

    {
      did = database.add_document(document_in);
      TEST_EQUAL(did, 1);
      TEST_EQUAL(database.get_doccount(), 1);
      TEST_EQUAL(database.get_avlength(), 2);
    }

    {
      document_in.remove_term("foo");
      document_in.add_posting("baz", 1);

      database.replace_document(1, document_in);

      database.delete_document(1);

      TEST_EQUAL(database.get_doccount(), 0);
      TEST_EQUAL(database.get_avlength(), 0);
      TEST_EQUAL(database.get_termfreq("foo"), 0);
      TEST_EQUAL(database.get_collection_freq("foo"), 0);
      TEST_EQUAL(database.get_termfreq("bar"), 0);
      TEST_EQUAL(database.get_collection_freq("bar"), 0);
      TEST_EQUAL(database.get_termfreq("baz"), 0);
      TEST_EQUAL(database.get_collection_freq("baz"), 0);
    }
  }

  {
    TEST_EQUAL(db.get_doccount(), 0);
    db.add_document(Xapian::Document());
    TEST_EQUAL(db.get_doccount(), 1);
    db.add_document(Xapian::Document());
    TEST_EQUAL(db.get_doccount(), 2);
  }

  {
    for (Xapian::doccount i = 0; i < 2100; ++i) {
      Xapian::Document doc;
      for (Xapian::termcount t = 0; t < 100; ++t) {
        string term("foo");
        term += char(t ^ 70 ^ i);
        doc.add_posting(term, t);
      }
      db.add_document(doc);
    }
    TEST(db.term_exists("foo1"));
  }
  return true;
}
