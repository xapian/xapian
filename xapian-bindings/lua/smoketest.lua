#!/usr/bin/env lua
--
-- Simple test to ensure that we can load the xapian module and exercise
-- basic functionality successfully.
--
-- Copyright (C) 2011 Xiaona Han
-- Copyright (C) 2011,2014 Olly Betts
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License as
-- published by the Free Software Foundation; either version 2 of the
-- License, or (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
-- USA

---
-- got: obtained value
-- exp: expected value
-- msg: failure message (a string)
function expect(got, exp, msg)
  msg = msg and msg..': ' or ''
  if type(exp) == "table" then
    assert('table' == type(got), msg..'not a table')
    assert(#exp == #got, msg..'got: '..tostring(#got)..' want: '..tostring(#exp))
    for i = 1, #exp do
      expect(got[i], exp[i])
    end
  else
    assert(exp == got, msg..'got: '..(tostring(got) or '???')..' want: '..(tostring(exp) or '???'))
  end
end

function run_tests()
  local xap = require 'xapian'
  -- Check that require sets global "xapian":
  expect(nil ~= xapian, true, 'global xapian is nil')
  expect(type(xapian), 'table', 'global xapian is not a table')
  -- Check that require returns the module table (SWIG 2.0.4 returned 'xapian'):
  expect(nil ~= xap, true, "require 'xapian' returned nil")
  expect(type(xap), 'table', "require 'xapian' didn't return a table")
  expect(xapian, xap, "require 'xapian' return value not the same as global xapian")

  stem = xapian.Stem("english")
  doc = xapian.Document()
  doc:set_data("is there anybody out there?")
  doc:add_posting(stem("is"), 1)
  doc:add_posting(stem("there"), 2)
  doc:add_posting(stem("anybody"), 3)
  doc:add_posting(stem("out"), 4)
  doc:add_posting(stem("there"), 5)
  doc:add_term("XYzzy")
  db = xapian.inmemory_open()
  db2 = xapian.inmemory_open()
  db:add_document(doc)
  enq = xapian.Enquire(db)

  it = db:positionlist_begin(1, "is")
  _end = db:positionlist_end(1, "is")
  while not it:equals(_end) do
    assert(it:get_termpos(), 1)
    it:next()
  end

  -- Test the version number reporting functions give plausible results.
  v = xapian.major_version() .. "." .. xapian.minor_version() .. "." .. xapian.revision()
  v2 = xapian.version_string()
  expect(v, v2)

  -- Test stem
  expect(tostring(stem), "Xapian::Stem(english)")
  expect(stem("is"), "is", 'stem noop "is" fails')
  expect(stem("going"), "go", 'stem noop "go" fails')
  expect(stem("wanted"), "want", 'stem(english) "wanted" -> "want" fails')
  expect(stem("reference"), "refer", 'stem(english) "reference" -> "refer" fails')

  -- Test document
  expect(doc:termlist_count(), 5)
  expect(doc:get_data(), "is there anybody out there?")
  doc:add_term("foo")
  expect(doc:termlist_count(), 6)

  -- Test database
  expect(tostring(db), "WritableDatabase()")
  expect(db:get_doccount(), 1, 'db should have 1 document')

  term_count = 0
  for term in db:allterms() do
   term_count = term_count + 1
  end
  expect(term_count, 5)

  -- Test queries
  terms = {"smoke", "test", "terms"}
  expect(tostring(xapian.Query(xapian.Query_OP_OR, terms)), "Xapian::Query((smoke OR test OR terms))")
  query1 = xapian.Query(xapian.Query_OP_PHRASE, {"smoke", "test", "tuple"})
  query2 = xapian.Query(xapian.Query_OP_XOR, {xapian.Query("smoke"), query1, "string"})
  expect(tostring(query1), "Xapian::Query((smoke PHRASE 3 test PHRASE 3 tuple))")
  expect(tostring(query2), "Xapian::Query((smoke XOR (smoke PHRASE 3 test PHRASE 3 tuple) XOR string))")
  subqs = {"a", "b"}
  expect(tostring(xapian.Query(xapian.Query_OP_OR, subqs)), "Xapian::Query((a OR b))")
  expect(tostring(xapian.Query(xapian.Query_OP_VALUE_RANGE, 0, '1', '4')), "Xapian::Query(VALUE_RANGE 0 1 4)")

  -- Test MatchAll and MatchNothing
  expect(tostring(xapian.Query_MatchAll), "Xapian::Query(<alldocuments>)")
  expect(tostring(xapian.Query_MatchNothing), "Xapian::Query()")

  -- Test enq
  query = xapian.Query(xapian.Query_OP_OR, {"there", "is"})
  enq:set_query(query)
  mset = enq:get_mset(0, 10)
  expect(mset:size(), 1)

  terms = {}
  for term in enq:get_matching_terms(mset:get_hit(0)) do
    table.insert(terms, term:get_term())
  end
  expect(terms, {"is", "there"})

  -- Check value of OP_ELITE_SET
  expect(xapian.Query_OP_ELITE_SET, 10)

  -- Test for MatchDecider
  doc = xapian.Document()
  doc:set_data("Two")
  doc:add_posting(stem("out"), 1)
  doc:add_posting(stem("outside"), 1)
  doc:add_posting(stem("source"), 2)
  doc:add_value(0, "yes");
  db:add_document(doc)

  function testmatchdecider(doc)
    return doc:get_value(0) == "yes"
  end

  query = xapian.Query("out")
  enq = xapian.Enquire(db)
  enq:set_query(query)
  mset = enq:get_mset(0, 10, None, testmatchdecider)
  expect(mset:size(), 1)
  expect(mset:get_docid(0), 2)

  rset = xapian.RSet()
  rset:add_document(1)
  eset = enq:get_eset(10, rset)
  expect(eset:size(), 4)

  eterms={}
  for term in eset:terms() do
    table.insert(eterms, term:get_term())
  end
  expect(eterms, {"there", "is", "anybodi", "XYzzy"})

  function testexpanddecider(term)
    return term ~= "there"
  end

  eset = enq:get_eset(10, rset, testexpanddecider)
  expect(eset:size(), 3)

  eterms={}
  for term in eset:terms() do
    table.insert(eterms, term:get_term())
  end
  expect(eterms, {"is", "anybodi", "XYzzy"})

  -- Regression test - overload resolution involving boolean types failed.
  enq:set_sort_by_value(1, true)

  -- Regression test - fixed in 0.9.10.1.
  oqparser = xapian.QueryParser()
  oquery = oqparser:parse_query("I like tea")

  -- Regression test for bug#192 - fixed in 1.0.3.
  enq:set_cutoff(100)

  -- Check DateValueRangeProcessor works
  qp = xapian.QueryParser()
  vrpdate = xapian.DateValueRangeProcessor(1, true, 1960)
  qp:add_valuerangeprocessor(vrpdate)
  query = qp:parse_query("12/03/99..12/04/01")
  expect(tostring(query), "Xapian::Query(VALUE_RANGE 1 19991203 20011204)")

  -- Test setting and getting metadata
  db:set_metadata('Foo', 'Foo')
  expect(db:get_metadata('Foo'), 'Foo')

  -- Test OP_SCALE_WEIGHT and corresponding constructor
  query4 = xapian.Query(xapian.Query_OP_SCALE_WEIGHT, xapian.Query('foo'), 5.0)
  expect(tostring(query4), "Xapian::Query(5 * foo)")

  -- Function to test the order of mset docid
  function mset_expect_order(mset, a)
    if mset:size() ~= #a then
      print(string.format("MSet has %i entries, expected %i\n",  mset:size(), #a))
      os.exit(-1)
    end

    for j = 0, #a -1 do
      hit = mset:get_hit(j)
      if hit:get_docid() ~= a[j + 1] then
        print(string.format("Expected MSet[%i] to be %i, got %i.\n", j, a[j + 1], hit:get_docid()))
        os.exit(-1)
      end
    end
  end

  -- Test MultiValueKeyMaker
  doc = xapian.Document()
  doc:add_term("foo")
  doc:add_value(0, "ABB")
  db2:add_document(doc)
  doc:add_value(0, "ABC")
  db2:add_document(doc)
  doc:add_value(0, "ABC\0")
  db2:add_document(doc)
  doc:add_value(0, "ABCD")
  db2:add_document(doc)
  doc:add_value(0, "ABC\xff")
  db2:add_document(doc)

  enq = xapian.Enquire(db2)
  enq:set_query(xapian.Query("foo"))

  sorter = xapian.MultiValueKeyMaker()
  sorter:add_value(0)
  enq:set_sort_by_key(sorter, true)
  mset = enq:get_mset(0, 10)
  mset_expect_order(mset, {5, 4, 3, 2, 1})

  sorter = xapian.MultiValueKeyMaker()
  sorter:add_value(0, true)
  enq:set_sort_by_key(sorter, true)
  mset = enq:get_mset(0, 10)
  mset_expect_order(mset, {1, 2, 3, 4, 5})

  sorter = xapian.MultiValueKeyMaker()
  sorter:add_value(0)
  sorter:add_value(1)
  enq:set_sort_by_key(sorter, true)
  mset = enq:get_mset(0, 10)
  mset_expect_order(mset, {5, 4, 3, 2, 1})

  sorter = xapian.MultiValueKeyMaker()
  sorter:add_value(0, true)
  sorter:add_value(1)
  enq:set_sort_by_key(sorter, true)
  mset = enq:get_mset(0, 10)
  mset_expect_order(mset, {1, 2, 3, 4, 5})

  sorter = xapian.MultiValueKeyMaker()
  sorter:add_value(0)
  sorter:add_value(1, true)
  enq:set_sort_by_key(sorter, true)
  mset = enq:get_mset(0, 10)
  mset_expect_order(mset, {5, 4, 3, 2, 1})

  sorter = xapian.MultiValueKeyMaker()
  sorter:add_value(0, true)
  sorter:add_value(1, true)
  enq:set_sort_by_key(sorter, true)
  mset = enq:get_mset(0, 10)
  mset_expect_order(mset, {1, 2, 3, 4, 5})

  -- Feature test for ValueSetMatchDecider:
  md = xapian.ValueSetMatchDecider(0, true)
  md:add_value("ABC")
  doc = xapian.Document()
  doc:add_value(0, "ABCD")
  if md(doc) then
    print "Unexpected result from ValueSetMatchDecider() expected false\n"
    os.exit(-1)
  end

  doc = xapian.Document()
  doc:add_value(0, "ABC")
  if not md(doc) then
    print "Unexpected result from ValueSetMatchDecider() expected true\n"
    os.exit(-1)
  end

  mset = enq:get_mset(0, 10, 0, nil, md)
  mset_expect_order(mset, {2})

  md = xapian.ValueSetMatchDecider(0, false)
  md:add_value("ABC")
  mset = enq:get_mset(0, 10, 0, nil, md)
  mset_expect_order(mset, {1, 3, 4, 5})

  -- Feature tests for Query "term" constructor optional arguments:
  query_wqf = xapian.Query('wqf', 3)
  if tostring(query_wqf) ~= 'Xapian::Query(wqf:(wqf=3))' then
    print "Unexpected query_wqf->tostring():\n"
    print(tostring(query_wqf) .."\n")
    os.exit(-1)
  end

  query = xapian.Query(xapian.Query_OP_VALUE_GE, 0, "100")
  if tostring(query) ~= 'Xapian::Query(VALUE_GE 0 100)' then
    print "Unexpected query->tostring():\n"
    print(tostring(query) .. "\n")
    os.exit(-1)
  end

  -- Test access to matchspy values:
  matchspy = xapian.ValueCountMatchSpy(0)
  enq:add_matchspy(matchspy)
  enq:get_mset(0, 10)
  beg = matchspy:values_begin()
  _end = matchspy:values_end()

  values = {}
  while not beg:equals(_end) do
    values[beg:get_term()] = beg:get_termfreq()
    beg:next()
  end

  expected = {["ABB"] = 1, ["ABC"] = 1, ["ABC\0"] = 1, ["ABCD"] =1, ["ABC\xff"] = 1}
  expect(values, expected)

  mset:get_hit(0)

  ---Test preservation of stopper set on query parser.
  function make_qp()
    queryparser = xapian.QueryParser()
    stopper = xapian.SimpleStopper()
    stopper:add('to')
    stopper:add('not')
    queryparser:set_stopper(stopper)
    return queryparser
  end
  queryparser = make_qp()
  query = queryparser:parse_query('to be')
  terms = {}
  for term in queryparser:stoplist() do
    table.insert(terms, term:get_term())
  end
  expect(terms, {"to"})

  -- Test preservation of stopper set on term generator.
  function make_tg()
    termgen = xapian.TermGenerator()
    termgen:set_stemmer(xapian.Stem('en'))
    stopper = xapian.SimpleStopper()
    stopper:add('to')
    stopper:add('not')
    termgen:set_stopper(stopper)
    return termgen
  end
  termgen = make_tg()
  termgen:index_text('to be')
  doc = termgen:get_document()
  terms = {}
  for term in doc:termlist() do
    table.insert(terms, term:get_term())
  end
  expect(terms, {"Zbe", "be", "to"})

  -- Test use of matchspies
  function setup_database()
    -- Set up and return an inmemory database with 5 documents.
    db = xapian.inmemory_open()

    doc = xapian.Document()
    doc:set_data("is it cold?")
    doc:add_term("is")
    doc:add_posting("it", 1)
    doc:add_posting("cold", 2)
    db:add_document(doc)

    doc = xapian.Document()
    doc:set_data("was it warm?")
    doc:add_posting("was", 1)
    doc:add_posting("it", 2)
    doc:add_posting("warm", 3)
    db:add_document(doc)
    doc:set_data("was it warm? two")
    doc:add_term("two", 2)
    doc:add_value(0, xapian.sortable_serialise(2))
    db:add_document(doc)
    doc:set_data("was it warm? three")
    doc:add_term("three", 3)
    doc:add_value(0, xapian.sortable_serialise(1.5))
    db:add_document(doc)
    doc:set_data("was it warm? four it")
    doc:add_term("four", 4)
    doc:add_term("it", 6)
    doc:add_posting("it", 7)
    doc:add_value(5, 'five')
    doc:add_value(9, 'nine')
    doc:add_value(0, xapian.sortable_serialise(2))
    db:add_document(doc)

    expect(db:get_doccount(), 5)
    return db
  end
  db = setup_database()
  query = xapian.Query(xapian.Query_OP_OR, "was", "it")
  enq = xapian.Enquire(db)
  enq:set_query(query)
  function set_matchspy_deref(enq)
    -- Set a matchspy, and then drop the reference, to check that it
    -- doesn't get deleted too soon.
    spy = xapian.ValueCountMatchSpy(0)
    enq:add_matchspy(spy)
  end
  set_matchspy_deref(enq)
  mset = enq:get_mset(0, 10)
  expect(mset:size(), 5)

  spy = xapian.ValueCountMatchSpy(0)
  enq:add_matchspy(spy)
  enq:clear_matchspies()
  mset = enq:get_mset(0, 10)
  spy:values()
  items = {}
  for item in spy:values() do
    table.insert(items, item:get_term())
  end
  expect(items, {})

  enq:add_matchspy(spy)
  mset = enq:get_mset(0, 10)
  expect(spy:get_total(), 5)
  items = {}
  for item in spy:values() do
    table.insert(items, {item:get_term(), item:get_termfreq()})
  end
  expect(items, {{xapian.sortable_serialise(1.5), 1}, {xapian.sortable_serialise(2), 2}})

  items = {}
  for item in spy:top_values(10) do
    table.insert(items, {item:get_term(), item:get_termfreq()})
  end
  expect(items, {{xapian.sortable_serialise(2), 2}, {xapian.sortable_serialise(1.5), 1}})

  -- Test exceptions
  ok,res = pcall(db.get_document, db, 0)
  expect(ok, false)
  expect(tostring(res), "InvalidArgumentError: Document ID 0 is invalid")
  expect(res:get_type(), "InvalidArgumentError")
end

ok, e = pcall(run_tests)
if not ok then
  print("Unexpected exception:", tostring(e))
  os.exit(-1)
end
