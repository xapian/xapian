# Simple command-line search program
#
# Copyright (C) 2004 Olly Betts
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
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA

# FIXME : path to xapian.so...
load [file join "../../.libs" xapian.so]

if {$argc < 2} {
    puts "usage: $argv0 <path to database> <search terms>"
    exit 1
}

if {[catch {
    set database [xapian::open [lindex $argv 0]]

    xapian::Enquire enquire $database
    xapian::Stem stemmer "english"
    xapian::Query firstterm [stemmer stem_word [lindex $argv 1]]
    set query firstterm
    for {set i 2} {$i < $argc} {incr i} {
	xapian::Query nextterm [stemmer stem_word [lindex $argv $i]]
	xapian::Query newquery 1 $query nextterm
	set query newquery
    }
    puts "Performing query `[$query get_description]'"

    enquire set_query $query
    set matches [enquire get_mset 0 10]
    puts "[$matches get_matches_estimated] results found"
    set i [$matches begin]
    while { ! [$i equals [$matches end]] } {
	xapian::Document document [$i get_document]
	puts "ID [$i get_docid] [$i get_percent]% \[[document get_data]\]"
	$i next
    }
} exception]} {
    puts stderr "Exception: $exception"
    exit 1
}
