#!/usr/bin/env ruby
#
# Simple command-line search program.
#
# Originally by Paul Legato (plegato@nks.net), 4/22/06
# Based on Python's simplesearch.py
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
  $stderr.puts "Usage: #{$0} <path to database> <search terms>"
  exit 99
end

database = Xapian::Database.new(ARGV[0])
enquire = Xapian::Enquire.new(database)
stemmer = Xapian::Stem.new("english")
terms = []
ARGV[1..-1].each {|term|
  terms.push(stemmer.stem_word(term.downcase))
}

query = Xapian::Query.new(Xapian::Query::OP_OR, terms)

puts "Performing query '#{query.description()}'..."

enquire.query = query
matchset = enquire.mset(0, 10)

puts "#{matchset.matches_estimated()} results found.\nMatches 1-#{matchset.size}:\n"

matchset.matches.each {|match|
  puts "docid #{match.docid}, weight #{match.weight} (#{match.percent}%), rank #{match.rank}, collapse count #{match.collapse_count}"
  puts "  Document contents: \n#{match.document.data}\n"
}
