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
#package require xapian 0.8.3
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
                doc set_data $para
                set pos 0
                # A term is one or more alphanumerics, with optional trailing
                # + and/or - (e.g. C++).  But "hyphen-ated" should generate
                # "hyphen" not "hyphen-".
                set re {([[:alnum:]]+(?:[-+]*(?![[:alnum:]]))?)}
                set j 0
                while {[regexp -indices -start $j $re $para -> word]} {
                    set i [lindex $word 0]
                    set j [lindex $word 1]
                    if {($j-$i) <= $MAX_PROB_TERM_LENGTH} {
                        set term [string range $para $i $j]
                        set term [string tolower $term]
                        set term [stemmer stem_word $term]
                        doc add_posting $term $pos
                        incr pos
                    }
                    incr j
                }
                database add_document doc
                set para ""
            }
        } else {
            if {[string equal $para ""]} {
                set para $line
            } else {
                set para "$para $line"
            }
        }
    }
    # Partial workaround for Tcl bindings not calling destructors.  However
    # a stale db_lock will still be left.
    # FIXME remove this once we work out what's going on there...
    database flush
} exception]} {
    puts stderr "Exception: $exception"
    exit 1
}
