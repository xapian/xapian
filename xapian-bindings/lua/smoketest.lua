#!/usr/bin/env lua
--
-- Simple test to ensure that we can load the xapian module and exercise
-- basic functionality successfully.
--
-- Copyright (C) 2011 Xiaona Han
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

require("xapian")
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
assert(v == v2)

-- Test stem
assert(stem:get_description(), "Xapian::Stem(english)")
assert("is" == stem("is"))
assert("go" == stem("going"))
assert("want" == stem("wanted"))
assert("refer" == stem("reference"))

-- Test document
assert(doc:termlist_count() == 5)
assert(doc:get_data() == "is there anybody out there?")
doc:add_term("foo")
assert(doc:termlist_count() == 6)

-- Test database
assert(db:get_description() == "WritableDatabase()")
assert(db:get_doccount() == 1)

term_count = 0
for term in db:allterms() do
 term_count = term_count + 1
end
assert(term_count == 5)

-- Test queries
terms = {"smoke", "test", "terms"}
assert(xapian.Query(xapian.Query_OP_OR, terms):get_description() == "Xapian::Query((smoke OR test OR terms))")
query1 = xapian.Query(xapian.Query_OP_PHRASE, {"smoke", "test", "tuple"})
query2 = xapian.Query(xapian.Query_OP_XOR, {xapian.Query("smoke"), query1, "string"})
assert(query1:get_description() == "Xapian::Query((smoke PHRASE 3 test PHRASE 3 tuple))")
assert(query2:get_description() == "Xapian::Query((smoke XOR (smoke PHRASE 3 test PHRASE 3 tuple) XOR string))")
subqs = {"a", "b"}
assert(xapian.Query(xapian.Query_OP_OR, subqs):get_description() == "Xapian::Query((a OR b))")
assert(xapian.Query(xapian.Query_OP_VALUE_RANGE, 0, '1', '4'):get_description() == "Xapian::Query(VALUE_RANGE 0 1 4)")

-- Test MatchAll and MatchNothing
assert(xapian.Query_MatchAll:get_description() == "Xapian::Query(<alldocuments>)")
assert(xapian.Query_MatchNothing:get_description() == "Xapian::Query()")

-- Test enq
query = xapian.Query(xapian.Query_OP_OR, {"there", "is"})
enq:set_query(query)
mset = enq:get_mset(0, 10)
assert(mset:size() == 1)

terms = {}
for term in enq:get_matching_terms(mset:get_hit(0)) do
	table.insert(terms, term:get_term())
end
assert(table.concat(terms, " ") == "is there")

-- Check value of OP_ELITE_SET
assert(xapian.Query_OP_ELITE_SET == 10)

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
assert(mset:size() == 1)
assert(mset:get_docid(0) == 2)

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
assert(query:get_description() == "Xapian::Query(VALUE_RANGE 1 19991203 20011204)")

-- Test setting and getting metadata
db:set_metadata('Foo', 'Foo')
assert(db:get_metadata('Foo') == 'Foo')

-- Test OP_SCALE_WEIGHT and corresponding constructor
query4 = xapian.Query(xapian.Query_OP_SCALE_WEIGHT, xapian.Query('foo'), 5.0)
assert(query4:get_description() == "Xapian::Query(5 * foo)")

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
if query_wqf:get_description() ~= 'Xapian::Query(wqf:(wqf=3))' then
  print "Unexpected \query_wqf->get_description():\n"
  print(query_wqf:get_description() .."\n")
  os.exit(-1)
end

query = xapian.Query(xapian.Query_OP_VALUE_GE, 0, "100")
if query:get_description() ~= 'Xapian::Query(VALUE_GE 0 100)' then
  print "Unexpected \query->get_description():\n"
  print(query:get_description() .. "\n")
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
assert(#values == #expected)
for i, v in ipairs(values) do
  assert(values[i] == expected[i])
end
mset:get_hit(0)
