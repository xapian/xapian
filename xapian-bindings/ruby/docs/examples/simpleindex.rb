#!/usr/bin/env ruby
#
# Index each paragraph of a textfile as a document.
#
# Originally by Paul Legato (plegato@nks.net), 4/22/06
# Based on Python's simplesearch.py
# Copyright (C) 2006 Networked Knowledge Systems, Inc.
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

MAX_PROB_TERM_LENGTH = 64
VERBOSE = false

if ARGV.size != 1
  $stderr.puts "Usage: #{$0} <path to database>"
  exit 99
end

# Extend String class with some helper methods
class String
  def alphanumeric?
    self =~ /[a-zA-Z0-9]/
  end

  def plusminus?
    self == '+' || self == '-'
  end

  # Scan through self. Return the offset of the first character for which the
  # given block is true, starting at start.
  # The block receives one character as its argument.
  #
  # Returns the length of the paragraph if the character is not found.
  #
  # N.B will not work properly on multibyte character encodings.
  #
  def find_p(start)
    self.split(//).each_with_index {|char, index|
      next if index < start
      return index if yield(char)
    }
    self.length
  end #find
end # class String

database = Xapian::WritableDatabase.new(ARGV[0], Xapian::DB_CREATE_OR_OPEN)
stemmer = Xapian::Stem.new("english")
para = ''

while line = $stdin.gets()
  line.strip!()
  if line.empty?
    if not para.empty?
      doc = Xapian::Document.new()
      doc.data = para
      pos = 0

      # At each point, find the next alnum character (i), then
      # find the first non-alnum character after that (j). Find
      # the first non-plusminus character after that (k), and if
      # k is non-alnum (or is off the end of the para), set j=k.
      # The term generation string is [i,j), so len = j-i
      i = 0
      while i < para.length
        i = para.find_p(i) {|x| x.alphanumeric? }
        j = para.find_p(i) {|x| !x.alphanumeric? }
        k = para.find_p(j) {|x| !x.plusminus? }

        j = k if k == para.length || !para[k].chr.alphanumeric?

        if (j - i) <= MAX_PROB_TERM_LENGTH and j > i
          term = stemmer.stem_word(para[i..j].strip.downcase)
          $stderr.puts " * Adding term #{term} at position #{pos}..." if VERBOSE
          doc.add_posting(term, pos)
          pos += 1
        end

        i = j
      end # while
      database.add_document(doc)
      para = ''
    end # if not para.empty?
  else # line not empty
    para += ' ' if para != ''
    para += line
  end # if line empty
end

