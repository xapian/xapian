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
db:add_document(doc)
enq = xapian.Enquire(db)

assert(stem:get_description(), "Xapian::Stem(english)")
assert("is" == stem("is"))
assert("go" == stem("going"))
assert("want" == stem("wanted"))
assert("refer" == stem("reference"))
assert(doc:termlist_count() == 5)
assert(doc:get_data() == "is there anybody out there?")

terms = {"smoke", "test", "terms"}
assert(xapian.Query(xapian.Query_OP_OR, terms):get_description() == "Xapian::Query((smoke OR test OR terms))")
query1 = xapian.Query(xapian.Query_OP_PHRASE, {"smoke", "test", "tuple"})
query2 = xapian.Query(xapian.Query_OP_XOR, {xapian.Query("smoke"), query1, "string"})
assert(query1:get_description() == "Xapian::Query((smoke PHRASE 3 test PHRASE 3 tuple))")
assert(query2:get_description() == "Xapian::Query((smoke XOR (smoke PHRASE 3 test PHRASE 3 tuple) XOR string))")
subqs = {"a", "b"}
assert(xapian.Query(xapian.Query_OP_OR, subqs):get_description() == "Xapian::Query((a OR b))")
assert(xapian.Query(xapian.Query_OP_VALUE_RANGE, 0, '1', '4'):get_description() == "Xapian::Query(VALUE_RANGE 0 1 4)")

query = xapian.Query(xapian.Query_OP_OR, {"there", "is"})
enq:set_query(query)
mset = enq:get_mset(0, 10)
assert(mset:size() == 1)

assert(table.concat(enq:get_matching_terms(mset:get_hit(0)), " ") == "is there")

assert(xapian.Query_OP_ELITE_SET == 10)
enq:set_sort_by_value(1,true)

oqparser = xapian.QueryParser()
oquery = oqparser:parse_query("I like tea")
enq:set_cutoff(100)

qp = xapian.QueryParser()
vrpdate = xapian.DateValueRangeProcessor(1, true, 1960)
qp:add_valuerangeprocessor(vrpdate)
query = qp:parse_query("12/03/99..12/04/01")
assert(query:get_description() == "Xapian::Query(VALUE_RANGE 1 19991203 20011204)")
db:set_metadata('Foo', 'Foo')
assert(db:get_metadata('Foo') == 'Foo')
query4 = xapian.Query(xapian.Query_OP_SCALE_WEIGHT, xapian.Query('foo'), 5.0)
assert(query4:get_description() == "Xapian::Query(5 * foo)")
