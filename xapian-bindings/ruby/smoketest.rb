#!/usr/bin/ruby -w
#
# smoketest.rb - test Xapian bindings for Ruby
# Original version by Paul Legato (plegato@nks.net), 4/17/2006
#
# Originally based on smoketest.php from the PHP4 bindings.
#
# Copyright (C) 2006 Networked Knowledge Systems, Inc.
# Copyright (C) 2008,2009,2010,2011,2016,2019,2025 Olly Betts
# Copyright (C) 2010 Richard Boulton
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
# USA

require 'test/unit'
require 'tmpdir'
require 'xapian'

class TestMatchDecider < Xapian::MatchDecider
  def __call__(doc)
    return doc.value(0) == "yes"
  end
end

class XapianSmoketest < Test::Unit::TestCase

  def setup
    @stem = Xapian::Stem.new("english")

    @doc = Xapian::Document.new()
    @doc.data = "is there anybody out there?"
    @doc.add_posting(@stem.call("is"), 1)
    @doc.add_posting(@stem.call("there"), 2)
    @doc.add_posting(@stem.call("anybody"), 3)
    @doc.add_posting(@stem.call("out"), 4)
    @doc.add_posting(@stem.call("there"), 5)
    @doc.add_term("XYzzy")

    @db = Xapian::inmemory_open()
    @db.add_document(@doc)

    @enq = Xapian::Enquire.new(@db)

    # Check Xapian::BAD_VALUENO is wrapped suitably.
    @enq.collapse_key = Xapian::BAD_VALUENO

    # Test that the non-constant wrapping prior to 1.4.10 still works.
    @enq.collapse_key = Xapian::BAD_VALUENO()
  end # setup

  def mset_expect_order(mset, a)
    if mset.size() != a.size
      puts "MSet has #{mset.size()} entries, expected #{a.size}"
      exit(1)
    end
    a.each_with_index { |expected, j|
      docid = mset.hit(j).docid
      if docid != expected
        puts "Expected MSet[#{j}] to be #{a[j]}, got #{docid}"
        exit(1)
      end
    }
  end # mset_expect_order

  def test_version
    # Test the version number reporting functions give plausible results.
    @v = sprintf("%d.%d.%d", Xapian::major_version(), Xapian::minor_version(),
                 Xapian::revision())
    @v2 = Xapian::version_string()
    assert_equal(@v2, @v)
  end # test_version

  def test_stem
    assert_equal("Xapian::Stem(english)", @stem.description())

    assert_equal("is", @stem.call("is"))
    assert_equal("go", @stem.call("going"))
    assert_equal("want", @stem.call("wanted"))
    assert_equal("refer", @stem.call("reference"))
  end # test_stem

  # subtests are those on which some test_foo() method depends.
  def test_000_document
    assert_not_nil(@doc)

    assert_equal("is there anybody out there?", @doc.data())

    assert_equal(@doc.termlist_count(), 5)
    assert_equal("XYzzy", @doc.terms().first.term)

    @doc.add_term("foo")
    assert_equal(6, @doc.termlist_count())
    assert_equal(@doc.terms.size(), @doc.termlist_count())

  end # test_document

  def test_001_database
    assert_not_nil(@db)
    assert_equal("WritableDatabase()", @db.description())
    assert_equal(1, @db.doccount())
  end # test_database

  def test_002_queries
    assert_equal("Query((smoke OR test OR terms))",
                 Xapian::Query.new(Xapian::Query::OP_OR, ["smoke", "test", "terms"]).description())

    phrase_query = Xapian::Query.new(Xapian::Query::OP_PHRASE, ["smoke", "test", "tuple"])
    xor_query = Xapian::Query.new(Xapian::Query::OP_XOR, [ Xapian::Query.new("smoke"), phrase_query, "string" ])

    assert_equal("Query((smoke PHRASE 3 test PHRASE 3 tuple))", phrase_query.description())
    assert_equal("Query((smoke XOR (smoke PHRASE 3 test PHRASE 3 tuple) XOR string))", xor_query.description())

    assert_equal([Xapian::Term.new("smoke", 1),
                  Xapian::Term.new("string", 1),
                  Xapian::Term.new("test", 1),
                  Xapian::Term.new("tuple", 1)], xor_query.terms())
    assert_equal(xor_query.terms(), xor_query.unique_terms())

    non_unique_query = Xapian::Query.new(Xapian::Query::OP_PHRASE,
                                         [Xapian::Query.new("the", 1, 1),
                                          Xapian::Query.new("the", 1, 2)])
    assert_equal([Xapian::Term.new("the", 1),
                  Xapian::Term.new("the", 1)], non_unique_query.terms())
    assert_equal([Xapian::Term.new("the", 1)], non_unique_query.unique_terms())

    count = 0
    non_unique_query.terms() { count += 1 }
    assert_equal(2, count)

    count = 0
    non_unique_query.unique_terms() { count += 1 }
    assert_equal(1, count)

    assert_equal(Xapian::Query::OP_ELITE_SET, 10)

    assert_equal("Query(<alldocuments>)", Xapian::Query::MatchAll.description())
    assert_equal("Query()", Xapian::Query::MatchNothing.description())
  end # test_queries

  def test_003_enquire
    @enq = Xapian::Enquire.new(@db)
    assert_not_nil(@enq)

    @enq.query = Xapian::Query.new(Xapian::Query::OP_OR, "there", "is")
    mset = @enq.mset(0, 10)

    assert_equal(1, mset.size())

    # Feature test for Enquire.matching_terms()
    assert_equal(2, @enq.matching_terms(mset.hit(0)).size())
    assert_equal([Xapian::Term.new("is", 1), Xapian::Term.new("there", 1)],
                 @enq.matching_terms(mset.hit(0)))
  end # test_enquire

  def test_004_mset_iterator
    @enq = Xapian::Enquire.new(@db)
    assert_not_nil(@enq)

    @enq.query = Xapian::Query.new(Xapian::Query::OP_OR, "there", "is")
    mset = @enq.mset(0, 10)

    assert_equal(mset.matches().size(), mset.size())
    msize = 0
    mset.matches { |x| msize += 1 }
    assert_equal(msize, mset.size())
  end


  def test_005_eset_iterator
    rset = Xapian::RSet.new

    rset.add_document(1)

    @enq = Xapian::Enquire.new(@db)
    @enq.query = Xapian::Query.new(Xapian::Query::OP_OR, "there", "is")

    eset = @enq.eset(10, rset)
    assert_not_nil(eset)

    assert_equal(3, eset.terms.size())
    assert_equal(3, eset.size())
    esize = 0
    eset.terms { |x| esize += 1 }
    assert_equal(3, esize)
  end # test_eset_iter

  # Feature test for Database.allterms
  def test_006_database_allterms
    assert_equal(5, @db.allterms.size())
    ou_terms = @db.allterms('ou')
    assert_equal(1, ou_terms.size())
    assert_equal('out', ou_terms[0].term)
    count = 0
    @db.allterms('ou') { |t|
      count += 1
      assert_equal(t.term, "out")
    }
    assert_equal(1, count)
  end

  # Feature test for Database.postlist
  def test_007_database_postlist
    assert_equal(1, @db.postlist("there").size())
    count = 0
    @db.postlist("there") { |x| count += 1 }
    assert_equal(1, count)
  end

  # Feature test for Database.termlist
  def test_008_database_termlist
    assert_equal(5, @db.termlist(1).size())
    count = 0
    @db.termlist(1) { |t| count += 1 }
    assert_equal(5, count)
  end

  # Feature test for Database.positionlist
  def test_009_database_positionlist
    assert_equal(2, @db.positionlist(1, "there").size())
    count = 0
    @db.positionlist(1, "there") { |x| count += 1 }
    assert_equal(2, count)
  end

  # Feature test for Document.values
  def test_010_document_values
    assert_equal(0, @doc.values().size())
    @doc.values() { |x| assert(false) }
  end

  def test_011_matchdecider
    @doc = Xapian::Document.new()
    @doc.data = "Two"
    @doc.add_posting(@stem.call("out"), 1)
    @doc.add_posting(@stem.call("source"), 2)
    @doc.add_value(0, "yes")
    @db.add_document(@doc)

    @query = Xapian::Query.new(@stem.call("out"))
    enquire = Xapian::Enquire.new(@db)
    enquire.query = @query
    mset = enquire.mset(0, 10, nil, TestMatchDecider.new)
    assert_equal(mset.size(), 1)
    assert_equal(mset.docid(0), 2)
  end

  def test_012_metadata
    assert_equal(@db.get_metadata('Foo'), '')
    @db.set_metadata('Foo', 'Foo')
    assert_equal(@db.get_metadata('Foo'), 'Foo')

    # The inmemory backend doesn't support metadata_keys so we need to create a
    # "real" database for these tests.
    Dir.mktmpdir("smokerb") {|tmpdir|
        dbpath = "#{tmpdir}/db"

        db = Xapian::WritableDatabase.new(dbpath, Xapian::DB_CREATE_OR_OVERWRITE)
        assert_equal(db.get_metadata('Foo'), '')
        db.set_metadata('Foo', 'Foo')
        assert_equal(db.get_metadata('Foo'), 'Foo')
        assert_equal(db.metadata_keys(), ["Foo"])
        assert_equal(db.metadata_keys('F'), ["Foo"])
        assert_equal(db.metadata_keys('Foo'), ["Foo"])
        assert_equal(db.metadata_keys('A'), [])
        assert_equal(db.metadata_keys('Food'), [])
        assert_equal(db.metadata_keys('f'), [])
        count = 0
        db.metadata_keys { |k|
          count += 1
          assert_equal(k, "Foo")
        }
        assert_equal(1, count)
    }
  end

  def test_013_scaleweight
    query = Xapian::Query.new("foo")
    query2 = Xapian::Query.new(Xapian::Query::OP_SCALE_WEIGHT, query, 5)
    assert_equal(query2.description(), "Query(5 * foo)")
  end

  def test_014_multivaluekeymaker
    db2 = Xapian::WritableDatabase.new('', Xapian::DB_BACKEND_INMEMORY)
    doc = Xapian::Document.new()
    doc.add_term("foo")
    doc.add_value(0, "ABB")
    db2.add_document(doc)
    doc.add_value(0, "ABC")
    db2.add_document(doc)
    doc.add_value(0, "ABC\0")
    db2.add_document(doc)
    doc.add_value(0, "ABCD")
    db2.add_document(doc)
    doc.add_value(0, "ABC\xff")
    db2.add_document(doc)

    enquire = Xapian::Enquire.new(db2)
    enquire.query = Xapian::Query.new("foo")

    begin
      sorter = Xapian::MultiValueKeyMaker.new()
      sorter.add_value(0)
      enquire.set_sort_by_key(sorter, true)
      mset = enquire.mset(0, 10)
      mset_expect_order(mset, [5, 4, 3, 2, 1])
    end

    begin
      sorter = Xapian::MultiValueKeyMaker.new()
      sorter.add_value(0, true)
      enquire.set_sort_by_key(sorter, true)
      mset = enquire.mset(0, 10)
      mset_expect_order(mset, [1, 2, 3, 4, 5])
    end

    begin
      sorter = Xapian::MultiValueKeyMaker.new()
      sorter.add_value(0)
      sorter.add_value(1)
      enquire.set_sort_by_key(sorter, true)
      mset = enquire.mset(0, 10)
      mset_expect_order(mset, [5, 4, 3, 2, 1])
    end

    begin
      sorter = Xapian::MultiValueKeyMaker.new()
      sorter.add_value(0, true)
      sorter.add_value(1)
      enquire.set_sort_by_key(sorter, true)
      mset = enquire.mset(0, 10)
      mset_expect_order(mset, [1, 2, 3, 4, 5])
    end

    begin
      sorter = Xapian::MultiValueKeyMaker.new()
      sorter.add_value(0)
      sorter.add_value(1, true)
      enquire.set_sort_by_key(sorter, true)
      mset = enquire.mset(0, 10)
      mset_expect_order(mset, [5, 4, 3, 2, 1])
    end

    begin
      sorter = Xapian::MultiValueKeyMaker.new()
      sorter.add_value(0, true)
      sorter.add_value(1, true)
      enquire.set_sort_by_key(sorter, true)
      mset = enquire.mset(0, 10)
      mset_expect_order(mset, [1, 2, 3, 4, 5])
    end
  end

  def test_015_sortable_serialise
    # In Xapian 1.0.13/1.1.1 and earlier, the SWIG generated wrapper code
    # didn't handle integer values > MAXINT for double parameters.
    v = 51767811298
    assert_equal(v, Xapian::sortable_unserialise(Xapian::sortable_serialise(v)))
  end

  def test_016_valuecount_matchspy
    spy = Xapian::ValueCountMatchSpy.new(0)
    doc = Xapian::Document.new()
    doc.add_posting("term", 1)
    doc.add_value(0, "yes")
    @db.add_document(doc)
    @db.add_document(doc)
    doc.add_value(0, "maybe")
    @db.add_document(doc)
    doc.add_value(0, "no")
    @db.add_document(doc)
    query = Xapian::Query.new("term")
    enquire = Xapian::Enquire.new(@db)
    enquire.query = query
    enquire.add_matchspy(spy)
    mset = enquire.mset(0, 10)
    assert_equal(mset.size(), 4)
    assert_equal(spy.values.map{|i| "%s:%d"%[i.term, i.termfreq]} * ",",
                 "maybe:1,no:1,yes:2")
    assert_equal(spy.top_values(1).map{|i| "%s:%d"%[i.term, i.termfreq]} * ",",
                 "yes:2")
    assert_equal(spy.top_values(2).map{|i| "%s:%d"%[i.term, i.termfreq]} * ",",
                 "yes:2,maybe:1")
    assert_equal(spy.top_values(3).map{|i| "%s:%d"%[i.term, i.termfreq]} * ",",
                 "yes:2,maybe:1,no:1")

    # Test the valuestream iterator, while we've got some data
    assert_equal(@db.valuestream(1).size(), 0)
    assert_equal(@db.valuestream(0).map{|i| "%d:%s"%[i.docid, i.value]}*",",
                 "2:yes,3:yes,4:maybe,5:no")
  end

  def test_017_compactor
    Dir.mktmpdir("smokerb") {|tmpdir|
        db1path = "#{tmpdir}/db1"
        db2path = "#{tmpdir}/db2"
        db3path = "#{tmpdir}/db3"

        # Set up a couple of sample input databases
        db1 = Xapian::WritableDatabase.new(db1path, Xapian::DB_CREATE_OR_OVERWRITE)
        doc1 = Xapian::Document.new()
        doc1.add_term('Hello')
        doc1.add_term('Hello1')
        doc1.add_value(0, 'Val1')
        db1.set_metadata('key', '1')
        db1.set_metadata('key1', '1')
        db1.add_document(doc1)
        db1.commit()

        db2 = Xapian::WritableDatabase.new(db2path, Xapian::DB_CREATE_OR_OVERWRITE)
        doc2 = Xapian::Document.new()
        doc2.add_term('Hello')
        doc2.add_term('Hello2')
        doc2.add_value(0, 'Val2')
        db2.set_metadata('key', '2')
        db2.set_metadata('key2', '2')
        db2.add_document(doc2)
        db2.commit()

        # Compact with the default compactor
        # Metadata conflicts are resolved by picking the first value
        c = Xapian::Compactor.new()
        c.add_source(db1path)
        c.add_source(db2path)
        c.set_destdir(db3path)
        c.compact()

        db3 = Xapian::Database.new(db3path)
        #assert_equal([(item.term, item.termfreq) for item in db3.allterms()],
        #       [('Hello', 2), ('Hello1', 1), ('Hello2', 1)])
        assert_equal(db3.document(1).value(0), 'Val1')
        assert_equal(db3.document(2).value(0), 'Val2')
        assert_equal(db3.get_metadata('key'), '1')
        assert_equal(db3.get_metadata('key1'), '1')
        assert_equal(db3.get_metadata('key2'), '2')
    }
  end

  def test_018_latlongcoords_iterator
    coords = Xapian::LatLongCoords.new()
    coords.append(Xapian::LatLongCoord.new(0, 0))
    assert_equal(coords.size(), 1)
    assert_equal(coords.all.map{|i| "%s"%i.description}*",",
                 "Xapian::LatLongCoord(0, 0)")
    s = ''
    coords.all {|i| s += i.description }
    assert_equal(s, "Xapian::LatLongCoord(0, 0)")
  end

  def test_019_spellings
    # The inmemory backend doesn't support spellings so we need to create a
    # "real" database for these tests.
    Dir.mktmpdir("smokerb") {|tmpdir|
        dbpath = "#{tmpdir}/dbspell"

        db = Xapian::WritableDatabase.new(dbpath, Xapian::DB_CREATE_OR_OVERWRITE)
        a = []
        db.spellings { |x| a.push(x) }
        assert_equal(a, db.spellings)
        assert_equal([], a)
        db.add_spelling("there")
        db.add_spelling("their")
        db.add_spelling("they're")
        db.add_spelling("there")
        a = []
        db.spellings { |x| a.push(x) }
        assert_equal(a, db.spellings)
        assert_equal([Xapian::Term.new("their", 0, 1),
                      Xapian::Term.new("there", 0, 2),
                      Xapian::Term.new("they're", 0, 1)], a)
    }
  end

  def test_020_synonyms
    # The inmemory backend doesn't support synonyms so we need to create a
    # "real" database for these tests.
    Dir.mktmpdir("smokerb") {|tmpdir|
        dbpath = "#{tmpdir}/dbsynonym"

        db = Xapian::WritableDatabase.new(dbpath, Xapian::DB_CREATE_OR_OVERWRITE)

        k = []
        db.synonym_keys { |x| k.push(x) }
        assert_equal(k, db.synonym_keys)
        assert_equal(k, [])

        a = []
        db.synonyms('food') { |x| a.push(x) }
        assert_equal(a, db.synonyms('food'))
        assert_equal(a, [])

        db.add_synonym("food", "nosh")
        db.add_synonym("food", "grub")
        db.add_synonym("food", "kai")
        db.add_synonym("drink", "tea")
        db.add_synonym("drink", "coffee")

        k = []
        db.synonym_keys { |x| k.push(x) }
        assert_equal(k, db.synonym_keys)
        assert_equal(['drink', 'food'], k)

        a = []
        db.synonyms('drink') { |x| a.push(x) }
        assert_equal(a, db.synonyms('drink'))
        assert_equal(['coffee', 'tea'], a)

        a = []
        db.synonyms('food') { |x| a.push(x) }
        assert_equal(a, db.synonyms('food'))
        assert_equal(['grub', 'kai', 'nosh'], a)

        a = []
        db.synonyms('nothing') { |x| a.push(x) }
        assert_equal(a, db.synonyms('nothing'))
        assert_equal([], a)
    }
  end

  def test_021_queryparser
    stopper = Xapian::SimpleStopper.new()
    stopper.add('a')
    stopper.add('the')
    qp = Xapian::QueryParser.new()
    qp.stopper = stopper
    qp.stemmer = Xapian::Stem.new('en')
    q = qp.parse_query("The starting started with a start")
    puts q.description

    a = []
    qp.stoplist() { |x| a.push(x) }
    assert_equal(a, qp.stoplist())
    assert_equal(['the', 'a'], a)

    a = []
    qp.unstem('Zthe') { |x| a.push(x) }
    assert_equal(a, qp.unstem('Zthe'))
    assert_equal([], a)

    a = []
    qp.unstem('Zstart') { |x| a.push(x) }
    assert_equal(a, qp.unstem('Zstart'))
    assert_equal(['starting', 'started', 'start'], a)
  end

end # class XapianSmoketest
