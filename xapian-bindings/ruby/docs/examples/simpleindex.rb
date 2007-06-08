#!/usr/bin/env ruby
#
# Index each paragraph of a text file as a Xapian document.
#
# Originally by Paul Legato (plegato@nks.net), 4/22/06
# Based on Python's simplesearch.py
# Copyright (C) 2006 Networked Knowledge Systems, Inc.
# Copyright (C) 2007 Olly Betts
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

require 'xapian'

if ARGV.size != 1
  $stderr.puts "Usage: #{$0} PATH_TO_DATABASE"
  exit 99
end

# Open the database for update, creating a new database if necessary.
database = Xapian::WritableDatabase.new(ARGV[0], Xapian::DB_CREATE_OR_OPEN)

indexer = Xapian::TermGenerator.new()
stemmer = Xapian::Stem.new("english")
indexer.stemmer = stemmer

para = ''
while line = $stdin.gets()
  line.strip!()
  if line.empty?
    if not para.empty?
      # We've reached the end of a paragraph, so index it.
      doc = Xapian::Document.new()
      doc.data = para

      indexer.document = doc
      indexer.index_text(para)

      # Add the document to the database
      database.add_document(doc)
      para = ''
    end # if not para.empty?
  else # line not empty
    para += ' ' if para != ''
    para += line
  end # if line empty
end

