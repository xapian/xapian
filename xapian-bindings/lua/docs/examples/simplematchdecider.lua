#!/usr/bin/env lua
--
-- Simple command-line match decider example.
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

-- This example runs a query like simplesearch does, but uses a MatchDecider
-- (mymatchdecider) to discard any document for which value 0 is equal to
-- the string passed as the second command line argument.

if #arg < 3 then
	io.stderr:write("Usage:" .. arg[0] .. " PATH_TO_DATABASE AVOID_ID QUERY\n")
	os.exit()
end

-- My own matchdecider
function mymatchdecider(doc)
	return doc:get_docid() ~= avoid_id
end

-- Open the database for searching.
database = xapian.Database(arg[1])

-- Start an enquire session.
enquire = xapian.Enquire(database)

-- Save the second command line argument into the variable and combine the rest
-- of the command line arguments with spaces between them, so that simple
-- queries don't have to be quoted at the shell level.
avoid_id = tonumber(arg[2])
query_string = table.concat(arg, " ", 3, #arg)

-- Parse the query string to produce a Xapian::Query object.
qp = xapian.QueryParser()
stemmer = xapian.Stem("english")
qp:set_stemmer(stemmer)
qp:set_database(database)
qp:set_stemming_strategy(xapian.QueryParser_STEM_SOME)
query = qp:parse_query(query_string)
print("Parsed query is: " .. tostring(query))

-- Find the top 10 results for the query.
enquire:set_query(query)
matches = enquire:get_mset(0, 10, None, mymatchdecider)

-- Display the size of the results.
print(string.format("%u results found.", matches:get_matches_estimated()))
print(string.format("Results 1-%i:", matches:size()))

-- Display the results
for m in matches:items() do
	print(m:get_rank() + 1, m:get_percent() .. "%", m:get_docid(), m:get_document():get_data())
end
