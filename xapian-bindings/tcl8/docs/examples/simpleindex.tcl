#!/usr/bin/env tclsh
# Index each paragraph of a textfile as a document
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

# FIXME: sort out pkgIndex.tcl and then use something like:
# lappend auto_path /path/to/xapian/lib/with/the/package
# package require xapian 0.83
load [file join "../../.libs" xapian.so]

set MAX_PROB_TERM_LENGTH 64

if {[llength $argv] != 1} {
    puts "usage: $argv0 <path to database>"
    exit 1
}

if {[catch {
    xapian::WritableDatabase database \
        [xapian::open [lindex $argv 0] $DB_CREATE_OR_OPEN]
    xapian::Stem stemmer "english"

    set para ""
    while {![eof stdin]} {
        gets stdin line
        set line [string trim $line]
        if {[string equal $line ""]} {
            if {[string compare $para ""]} {
                xapian::Document doc
                doc set_data para
                set pos 0
                # A term is one or more alphanumerics, with optional trailing
                # + and/or - (e.g. C++)
                set re {([[:alnum:]]+[-+]*)}
                set i 0
                while {[regexp -indices -start $i $re $para -> word]} {
                    set j [lindex $word 1]
                    if {($j-$i) <= $MAX_PROB_TERM_LENGTH} {
                        set term [string range $para $i [expr {$j-1}]]
			puts $term
                        set term [string tolower $term]
                        set term [stemmer stem_word $term]
                        doc add_posting $term $pos
                        incr pos
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
