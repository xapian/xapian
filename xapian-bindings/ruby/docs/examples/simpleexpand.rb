#!/usr/bin/env ruby
#
# Simple example script demonstrating query expansion.
#
# Originally by Paul Legato (plegato@nks.net), 4/22/06.
#
# Copyright (C) 2006 Networked Knowledge Systems, Inc.
# Copyright (C) 2006,2007 Olly Betts
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

queryString = ''
relevantDocs = Xapian::RSet.new()
onDocIdsYet = false

# Combine the rest of the command line arguments with spaces between
# them, so that simple queries don't have to be quoted at the shell
# level.
ARGV.each_with_index { |arg,index|
  next if index == 0 # skip path to db

  if arg == '--'
    onDocIdsYet = true
    next
  end

  if onDocIdsYet
    relevantDocs.add_document(arg.to_i)
  else
    queryString += ' ' unless queryString.empty?
    queryString += arg
  end
}


# Parse the query string to produce a Xapian::Query object.
qp = Xapian::QueryParser.new()
stemmer = Xapian::Stem.new("english")
qp.stemmer = stemmer
qp.database = database
qp.stemming_strategy = Xapian::QueryParser::STEM_SOME
query = qp.parse_query(queryString)

unless query.empty?
  puts "Parsed query is: #{query.description()}"

  # Find the top 10 results for the query.
  enquire.query = query
  matchset = enquire.mset(0, 10, relevantDocs)

  # Display the results.
  puts "#{matchset.matches_estimated()} results found."
  puts "Matches 1-#{matchset.size}:\n"

  matchset.matches.each {|m|
    puts "#{m.rank + 1}: #{m.percent}% docid=#{m.docid} [#{m.document.data}]\n"
  }
end
  
# Put the top 5 (at most) docs into the rset if rset is empty
if relevantDocs.empty?
  matchset.matches[0..4].each {|match|
    relevantDocs.add_document(match.docid())
  }
end

# Get the suggested expand terms
expandTerms = enquire.eset(10, relevantDocs)
puts "#{expandTerms.size()} suggested additional terms:"
expandTerms.terms.each {|term|
  puts "  * Term \"#{term.name}\", weight #{term.weight}"
}
