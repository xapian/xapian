#!/usr/bin/env ruby
#
# Simple example script demonstrating query expansion.
#
# Originally by Paul Legato (plegato@nks.net), 4/22/06.
#
# Copyright (C) 2006 Networked Knowledge Systems, Inc.
# Copyright (C) 2006,2007,2016 Olly Betts
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

if ARGV.size < 2
  $stderr.puts "Usage: #{$0} PATH_TO_DATABASE QUERY [-- [DOCID...]]"
  exit 99
end

# Open the database for searching.
database = Xapian::Database.new(ARGV[0])

# Start an enquire session.
enquire = Xapian::Enquire.new(database)

query_string = ''
relevant_docs = Xapian::RSet.new()
on_doc_ids_yet = false

# Combine the rest of the command line arguments with spaces between
# them, so that simple queries don't have to be quoted at the shell
# level.
ARGV.each_with_index { |arg,index|
  next if index == 0 # skip path to db

  if arg == '--'
    on_doc_ids_yet = true
    next
  end

  if on_doc_ids_yet
    relevant_docs.add_document(arg.to_i)
  else
    query_string += ' ' unless query_string.empty?
    query_string += arg
  end
}


# Parse the query string to produce a Xapian::Query object.
qp = Xapian::QueryParser.new()
stemmer = Xapian::Stem.new("english")
qp.stemmer = stemmer
qp.database = database
qp.stemming_strategy = Xapian::QueryParser::STEM_SOME
query = qp.parse_query(query_string)

unless query.empty?
  puts "Parsed query is: #{query.description()}"

  # Find the top 10 results for the query.
  enquire.query = query
  matchset = enquire.mset(0, 10, relevant_docs)

  # Display the results.
  puts "#{matchset.matches_estimated()} results found."
  puts "Matches 1-#{matchset.size}:\n"

  matchset.matches.each {|m|
    puts "#{m.rank + 1}: #{m.percent}% docid=#{m.docid} [#{m.document.data}]\n"
  }
end

# Put the top 5 (at most) docs into the rset if rset is empty
if relevant_docs.empty?
  matchset.matches[0..4].each {|match|
    relevant_docs.add_document(match.docid())
  }
end

# Get the suggested expand terms
expand_terms = enquire.eset(10, relevant_docs)
puts "#{expand_terms.size()} suggested additional terms:"
expand_terms.terms.each {|term|
  puts "  * Term \"#{term.name}\", weight #{term.weight}"
}
