#!/usr/bin/env tclsh
# Index each paragraph of a textfile as a document
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

# We need at least Tcl version 8
package require Tcl 8

# FIXME: sort out pkgIndex.tcl and then use something like:
# lappend auto_path /path/to/xapian/lib/with/the/package
# package require xapian 0.83
load [file join "../../.libs" xapian.so]

set MAX_PROB_TERM_LENGTH 64

if {[llength $argv] != 1} {
    puts "usage: $argv0 <path to database>"
    exit 1
}

proc p_alnum {c} {
    return [string match {[A-Za-z0-9]} $c]
}

proc p_notalnum {c} {
    return [expr ! [p_alnum $c]]
}

proc p_notplusminus {c} {
    return [expr ! [string match {[-+]} $c]]
}

proc find_p {str i predicate} {
    set l [string length $str]
    while {$i < $l && ! [$predicate [string index $str $i]]} {
	set i [expr $i + 1]
    }
    return $i
}

if {[catch {
    xapian::WritableDatabase database \
	[xapian::open [lindex $argv 0] $DB_CREATE_OR_OPEN]
    xapian::Stem stemmer "english"

    set para ""
    while {![eof stdin]} {
	gets stdin line
	set line [string trim $line]
	if {$line eq ""} {
	    if {$para ne ""} {
		xapian::Document doc
		doc set_data para
		set pos 0
		# At each point, find the next alnum character (i), then
		# find the first non-alnum character after that (j). Find
		# the first non-plusminus character after that (k), and if
		# k is non-alnum (or is off the end of the para), set j=k.
		# The term generation string is [i,j), so len = j-i
		set i 0
		set l [string length $para]
		while {$i < $l} {
		    set i [find_p $para $i p_alnum]
		    set j [find_p $para $i p_notalnum]
		    set k [find_p $para $j p_notplusminus]
		    if {[p_notalnum [string index $para $k]]} {
			set j $k
		    }
		    if {[expr $j - $i] <= $MAX_PROB_TERM_LENGTH && $j > $i} {
			set term [string range $para $i [expr $j - 1]]
			set term [string tolower $term]
			set term [stemmer stem_word $term]
			doc add_posting $term $pos
			set pos [expr $pos + 1]
		    }
		    set i $j
		}
		database add_document doc
		set para ""
	    }
	} else {
	    if {$para ne ""} {
		set para "$para $line"
	    } else {
		set para $line
	    }
	}
    }
    # Partial workaround for Tcl bindings not calling destructors
    # FIXME remove this once we work out what's going on there...
    database flush
} exception]} {
    puts stderr "Exception: $exception"
    exit 1
}
