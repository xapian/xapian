#!/usr/bin/env ruby
#
# Simple command-line match decider example
#
# Originally by Paul Legato (plegato@nks.net), 4/22/06.
# Based on simplematchdecider.py.
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

if ARGV.size < 3
  puts "Usage: #{$0} <path to database> <avoid value> <search terms>"
  exit 99
end

class myMatcher < Xapian::MatchDecider
 # TODO: not implemented yet
end
