#!/usr/bin/env tclsh
# Simple command-line search program
#
# Copyright (C) 2004 Olly Betts
# Copyright (C) 2004 Michael Schlenker
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

# We need at least Tcl version 8.1
package require Tcl 8.1
#package require xapian 0.8.3
load [file join "../../.libs" xapian.so]

if {[llength $argv] < 2} {
    puts "usage: $argv0 <path to database> <search terms>"
    exit 1
}

if {[catch {
    set database [xapian::open [lindex $argv 0]]

    xapian::Enquire enquire $database
    xapian::Stem stemmer "english"

    xapian::Query firstterm [stemmer stem_word [lindex $argv 1]]
    set query firstterm
    foreach term [lrange $argv 2 end] {
        xapian::Query nextterm [stemmer stem_word $term]
        xapian::Query newquery $Query_OP_OR $query nextterm
        set query newquery
    }
    puts "Performing query `[$query get_description]'"

    enquire set_query $query
    set matches [enquire get_mset 0 10]
    puts "[$matches get_matches_estimated] results found"

    for {set i [$matches begin]} {![$i equals [$matches end]]} {$i next} {
        xapian::Document document [$i get_document]
        puts [format {ID %s %s%% [%s]} \
            [$i get_docid] [$i get_percent] [document get_data]]
    }
} exception]} {
    puts stderr "Exception: $exception"
    exit 1
}
