#!/usr/bin/env tclsh
# Simple command-line search Tcl script.
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
    puts "Usage: $argv0 PATH_TO_DATABASE QUERY"
    exit 1
}

if {[catch {
    xapian::Database database [lindex $argv 0]

    # Start an enquire session.
    xapian::Enquire enquire database

    # Combine the rest of the command line arguments with spaces between
    # them, so that simple queries don't have to be quoted at the shell
    # level.
    set query_string [join [lrange $argv 1 end]]
    xapian::QueryParser qp
    xapian::Stem stemmer "english"
    qp set_stemmer stemmer
    qp set_database database
    qp set_stemming_strategy $xapian::QueryParser_STEM_SOME
    set query [qp parse_query $query_string]
    puts "Parsed query is: [$query get_description]"

    # Find the top 10 results for the query.
    enquire set_query $query
    set matches [enquire get_mset 0 10]

    # Display the results.
    puts "[$matches get_matches_estimated] results found:"

    for {set i [$matches begin]} {![$i equals [$matches end]]} {$i next} {
	xapian::Document document [$i get_document]
	set rank [expr [$i get_rank] + 1]
	puts [format {%s: %s%% docid=%s [%s]} \
	    $rank [$i get_percent] [$i get_docid] [document get_data]]
    }
} exception]} {
    puts stderr "Exception: $errorCode $exception"
    exit 1
}
