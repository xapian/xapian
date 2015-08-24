#!/usr/bin/env lua
--
-- Simple example script demonstrating query expansion.
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

-- Require at least two command line arguments.
if #arg < 2 then
	io.stderr:write("Usage:" .. arg[0] .. " PATH_TO_DATABASE QUERY [-- [DOCID...]]\n")
	os.exit()
end

-- Open the database for searching.
database = xapian.Database(arg[1])

-- Start an enquire session.
enquire = xapian.Enquire(database)

-- Combine command line arguments up to "--" with spaces between
-- them, so that simple queries don't have to be quoted at the shell level.
query_string = arg[2]
local index = 3
while index <= #arg do
	local para = arg[index]
	index = index + 1;
	if para == '--' then
		break
	end
	query_string = query_string .. ' ' .. para
end

-- Create an RSet with the listed docids in.
reldocs = xapian.RSet()

--for index in range(index, #arg) do
for i = index, #arg do
	reldocs:add_document(tonumber(arg[index]))
	index = index + 1
end

-- Parse the query string to produce a Xapian::Query object.
qp = xapian.QueryParser()
stemmer = xapian.Stem("english")
qp:set_stemmer(stemmer)
qp:set_database(database)
qp:set_stemming_strategy(xapian.QueryParser_STEM_SOME)
query = qp:parse_query(query_string)

if not query:empty() then
	print("Parsed query is: " .. tostring(query))

	-- Find the top 10 results for the query.
	enquire:set_query(query)
	matches = enquire:get_mset(0, 10)

	-- Display the size of the results.
	print(string.format("%i results found.", matches:get_matches_estimated()))
	print(string.format("Results 1-%i:", matches:size()))

	-- Display the results
	for m in matches:items() do
		print(m:get_rank() + 1, m:get_percent() .. "%", m:get_docid(), m:get_document():get_data())
	end

	-- Put the top 5 (at most) docs into the rset if rset is empty
	if reldocs:empty() then
		local c = 5
		for m in matches:items() do
			reldocs:add_document(m:get_docid())
			print (c)
			c = c - 1
			if c == 0 then
				break
			end
		end
	end

	-- Get the suggested expand terms
	eterms = enquire:get_eset(10, reldocs)
	print (string.format("%i suggested additional terms", eterms:size()))
	for m in eterms:terms() do
		print(string.format("%s: %f", m:get_term(), m:get_weight()))
	end

end
