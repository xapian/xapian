#!/usr/bin/env ruby
#
# Simple command-line query expand program
#
# Originally by Paul Legato (plegato@nks.net), 4/22/06.
# Based on simpleexpand.py.
# Copyright (C) 2006 Networked Knowledge Systems, Inc.
# Copyright (C) 2006 Olly Betts
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
  $stderr.puts "Usage: #{$0} <path to database> <search terms> [-- <relevant docids>]"
  exit 99
end

database = Xapian::Database.new(ARGV[0])
enquire = Xapian::Enquire.new(database)
stemmer = Xapian::Stem.new("english")

terms = []
relevantDocs = Xapian::RSet.new()
onDocIdsYet = false

ARGV.each_with_index { |term,index|
  next if index == 0 # skip path to db

  if term == '--'
    onDocIdsYet = true
    next
  end

  onDocIdsYet ?  relevantDocs.add_document(term.to_i) : terms.push(term)
}

query = Xapian::Query.new(Xapian::Query::OP_OR, terms)

unless query.empty?
  puts "Performing query #{query.description()} against rset #{relevantDocs.description}..."

  enquire.query = query
  matchset = enquire.mset(0, 10, relevantDocs)

  puts "#{matchset.matches_estimated()} results found."
  matchset.matches.each {|match|
    puts "\n - #{match.to_s}: weight #{match.weight} (#{match.percent}%), rank #{match.rank}, collapse count #{match.collapse_count}, docid #{match.docid}"
    puts "  Document contents: \n#{match.document.data}\n"
  }
  
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

end # unless query.empty?

puts "\n * All done.\n"

