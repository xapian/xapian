#!/usr/bin/env tclsh
# Simple example Tcl script demonstrating query expansion.
#
# Copyright (C) 2004,2006,2007 Olly Betts
# Copyright (C) 2004 Michael Schlenker
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

# We need at least Tcl version 8.1.
package require Tcl 8.1
# We need only actually need Xapian 0.9.3 (for the query from list constructor
# wrapper), but "package require" doesn't accept differing major versions.
package require xapian 1.0.0

if {[llength $argv] < 2} {
    puts "Usage: $argv0 PATH_TO_DATABASE QUERY [-- [DOCID...]]"
    exit 1
}

if {[catch {
    xapian::Database database [lindex $argv 0]

    # Start an enquire session.
    xapian::Enquire enquire database

    # Combine command line arguments up to "--" with spaces between
    # them, so that simple queries don't have to be quoted at the shell
    # level.
    set args [lrange $argv 1 end]
    set sep [lsearch -exact $args "--"]

    if {$sep == -1} {
	set sep [llength $args]
    } else {
	incr sep -1
    }

    set query_string [join [lrange $args 0 $sep]]

    xapian::RSet rset
    incr sep 2
    foreach docid [lrange $args $sep end] {
	puts "$sep $docid"
	rset add_document $docid
    }

    xapian::QueryParser qp
    xapian::Stem stemmer "english"
    qp set_stemmer stemmer
    qp set_database database
    qp set_stemming_strategy $xapian::QueryParser_STEM_SOME
    set query [qp parse_query $query_string]
    puts "Parsed query is: [$query get_description]"

    # Find the top 10 results for the query.
    enquire set_query $query
    set matches [enquire get_mset 0 10 rset]

    # Display the results.
    puts "[$matches get_matches_estimated] results found:"

    for {set i [$matches begin]} {![$i equals [$matches end]]} {$i next} {
	xapian::Document document [$i get_document]
	set rank [expr [$i get_rank] + 1]
	puts [format {%s: %s%% docid=%s [%s]} \
	    $rank [$i get_percent] [$i get_docid] [document get_data]]
    }

    # If no relevant docids were given, invent an RSet containing the top 5
    # matches (or all the matches if there are less than 5).
    if {[rset empty]} {
	set c 5
	set i [$matches begin]
	while {$c > 0 && ![$i equals [$matches end]]} {
	    rset add_document [$i get_docid]
	    $i next
	    incr c -1
	}
    }

    # Generate an ESet containing terms that the user might want to add to
    # the query.
    xapian::ESet eset [enquire get_eset 10 rset]

    # List the terms.
    for {set t [eset begin]} {![$t equals [eset end]]} {$t next} {
	puts [format {%s: weight = %f} [$t get_term] [$t get_weight]]
    }
} exception]} {
    puts stderr "Exception: $errorCode $exception"
    exit 1
}
