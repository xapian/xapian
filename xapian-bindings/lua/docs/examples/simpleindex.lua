#!/usr/bin/env lua
--
-- Index each paragraph of a text file as a Xapian document.
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

-- Remove leading and trailing whitespace from a string
function trim(s)
  return (s:gsub("^%s*(.-)%s*$", "%1"))
end

-- Require one command line argument to mean the database directory
if #arg ~= 1 then
	io.stderr:write("Usage: " .. arg[0] .. " PATH_TO_DATABASE\n")
	os.exit()
end

-- Open the database for update, creating a new database if necessary.
database = xapian.WritableDatabase(arg[1], xapian.DB_CREATE_OR_OPEN)


indexer = xapian.TermGenerator()
stemmer = xapian.Stem("english")
indexer:set_stemmer(stemmer)

-- Read document data from standard input line.
-- An empty line means the end of a doc and the start of another doc
local para = ''
for line in io.lines() do
	if line == nil then
		break
	end

	line = trim(line)
	if line == '' then
		if para ~= '' then

			-- We've reached the end of a paragraph, so index it.
			doc = xapian.Document()
			doc:set_data(para)

			indexer:set_document(doc)
			indexer:index_text(para)

			-- Add the document to the database.
			database:add_document(doc)
			para = ''
		end
	else
		if para ~= '' then
			para = para .. ' '
		end
		para = para .. line
	end
end
