# uniParse.tcl --
#
#	This program parses the UnicodeData file and generates the
#	corresponding tclUniData.cc file with compressed character
#	data tables.  The input to this program should be the latest
#	UnicodeData file from:
#	    ftp://ftp.unicode.org/Public/UNIDATA/UnicodeData-Latest.txt
#
# Copyright (c) 1998-1999 by Scriptics Corporation.
# All rights reserved.
#
# RCS: @(#) $Id: uniParse.tcl,v 1.4 2001/05/28 04:37:57 hobbs Exp $


namespace eval uni {
    set shift 8; 		# number of bits of data within a page
				# This value can be adjusted to find the
				# best split to minimize table size

    variable pMap;		# map from page to page index, each entry is
				# an index into the pages table, indexed by
				# page number
    variable pages;		# map from page index to page info, each
				# entry is a list of indices into the groups
				# table, the list is indexed by the offset
    variable groups;		# list of character info values, indexed by
				# group number, initialized with the
				# unassigned character group

    variable categories {
	Cn Lu Ll Lt Lm Lo Mn Me Mc Nd Nl No Zs Zl Zp
	Cc Cf Co Cs Pc Pd Ps Pe Pi Pf Po Sm Sc Sk So
    };				# Ordered list of character categories, must
				# match the enumeration in the header file.

    variable titleCount 0;	# Count of the number of title case
				# characters.  This value is used in the
				# regular expression code to allocate enough
				# space for the title case variants.
}

proc uni::getValue {items index} {
    variable categories
    variable titleCount

    # Extract character info

    set category [lindex $items 2]
    if {[scan [lindex $items 12] %6x toupper] == 1} {
	set toupper [expr {$index - $toupper}]
    } else {
	set toupper {}
    }
    if {[scan [lindex $items 13] %6x tolower] == 1} {
	set tolower [expr {$tolower - $index}]
    } else {
	set tolower {}
    }
    if {[scan [lindex $items 14] %6x totitle] == 1} {
	set totitle [expr {$index - $totitle}]
    } else {
	set totitle {}
    }

    set categoryIndex [lsearch -exact $categories $category]
    if {$categoryIndex < 0} {
	puts "Unexpected character category: $index($category)"
	set categoryIndex 0
    } elseif {$category == "Lt"} {
	incr titleCount
    }

    return "$categoryIndex,$toupper,$tolower,$totitle"
}

proc uni::getGroup {value} {
    variable groups

    set gIndex [lsearch -exact $groups $value]
    if {$gIndex == -1} {
	set gIndex [llength $groups]
	lappend groups $value
    }
    return $gIndex
}

proc uni::addPage {info} {
    variable pMap
    variable pages

    set pIndex [lsearch -exact $pages $info]
    if {$pIndex == -1} {
	set pIndex [llength $pages]
	lappend pages $info
    }
    lappend pMap $pIndex
    return
}

proc uni::buildTables {data} {
    variable shift

    variable pMap {}
    variable pages {}
    variable groups {{0,,,}}
    set info {}			;# temporary page info

    set mask [expr {(1 << $shift) - 1}]

    set next 0

    foreach line [split $data \n] {
	if {$line == ""} {
	    set line "10FFFF;;Cn;0;ON;;;;;N;;;;;\n"
	}

	set items [split $line \;]

	scan [lindex $items 0] %6x index
	set index [format 0x%0.6x $index]

	set gIndex [getGroup [getValue $items $index]]

	# Since the input table omits unassigned characters, these will
	# show up as gaps in the index sequence.  There are a few special cases
	# where the gaps correspond to a uniform block of assigned characters.
	# These are indicated as such in the character name.

	# Enter all unassigned characters up to the current character.
	if {($index > $next) \
		&& ![regexp "Last>$" [lindex $items 1]]} {
	    for {} {$next < $index} {incr next} {
		lappend info 0
		if {($next & $mask) == $mask} {
		    addPage $info
		    set info {}
		}
	    }
	}

	# Enter all assigned characters up to the current character
	for {set i $next} {$i <= $index} {incr i} {
	    # Split character index into offset and page number
	    set offset [expr {$i & $mask}]
	    set page [expr {($i >> $shift)}]

	    # Add the group index to the info for the current page
	    lappend info $gIndex

	    # If this is the last entry in the page, add the page
	    if {$offset == $mask} {
		addPage $info
		set info {}
	    }
	}
	set next [expr {$index + 1}]
    }
    return
}

proc uni::main {} {
    global argc argv0 argv
    variable pMap
    variable pages
    variable groups
    variable shift
    variable titleCount

    if {$argc != 2} {
	puts stderr "\nusage: $argv0 <datafile> <outdir>\n"
	puts stderr "(<datafile> is UnicodeData-x.y.z.txt)\n"
	exit 1
    }

    if {![regexp {UnicodeData-(\d+\.\d+(\.\d+)?)\.txt$} [lindex $argv 0] dummy version]} {
	puts stderr "datafile name [lindex $argv 0] doesn't match UnicodeData-x.y.z.txt\n"
	exit 1
    }

    set f [open [lindex $argv 0] r]
    set data [read $f]
    close $f

    buildTables $data
    puts "X = [llength $pMap]  Y= [llength $pages]  A= [llength $groups]"
    set size [expr {[llength $pMap] + [llength $pages]*(1<<$shift)}]
    puts "shift = $shift, space = $size"
    puts "title case count = $titleCount"

    set f [open [file join [lindex $argv 1] tclUniData.cc] w]
    fconfigure $f -translation lf
    puts $f "/*
 * tclUniData.cc --
 *
 *	Declarations of Unicode $version character information tables.  This
 *	file is automatically generated by a modified version of the
 *	tools/uniParse.tcl script from the Tcl sources.
 *
 *	Do not modify this file by hand!
 *
 * Copyright (c) 1998 by Scriptics Corporation.
 * All rights reserved.
 */

#include <config.h>

#include <xapian/unicode.h>
#include \"omassert.h\"

/*
 * A 16-bit Unicode character is split into two parts in order to index
 * into the following tables.  The lower OFFSET_BITS comprise an offset
 * into a page of characters.  The upper bits comprise the page number.
 */

#define OFFSET_BITS $shift

/*
 * The pageMap is indexed by page number and returns an alternate page number
 * that identifies a unique page of characters.  Many Unicode characters map
 * to the same alternate page number.
 */

static const
unsigned char pageMap\[\] = {"
    set line "   "
    set last [expr {[llength $pMap] - 1}]
    for {set i 0} {$i <= $last} {incr i} {
	append line " "
	append line [lindex $pMap $i]
	if {$i != $last} {
	    append line ","
	}
	if {[string length $line] > 70} {
	    puts $f $line
	    set line "   "
	}
    }
    puts $f $line
    puts $f "};

/*
 * The groupMap is indexed by combining the alternate page number with
 * the page offset and returns a group number that identifies a unique
 * set of character attributes.
 */

static const
unsigned char groupMap\[\] = {"
    set line "   "
    set lasti [expr {[llength $pages] - 1}]
    for {set i 0} {$i <= $lasti} {incr i} {
	set page [lindex $pages $i]
	set lastj [expr {[llength $page] - 1}]
	for {set j 0} {$j <= $lastj} {incr j} {
	    append line " "
	    append line [lindex $page $j]
	    if {$j != $lastj || $i != $lasti} {
		append line ","
	    }
	    if {[string length $line] > 70} {
		puts $f $line
		set line "   "
	    }
	}
    }
    puts $f $line
    puts $f "};

/*
 * Each group represents a unique set of character attributes.  The attributes
 * are encoded into a 32-bit value as follows:
 *
 * Bits 0-4	Character category: see the constants listed below.
 *
 * Bits 5-7	Case delta type: 000 = identity
 *				 010 = add delta for lower
 *				 011 = add delta for lower, add 1 for title
 *				 100 = subtract delta for title/upper
 *				 101 = sub delta for upper, sub 1 for title
 *				 110 = sub delta for upper, add delta for lower
 *
 * Bits 8-14	Reserved for future use.
 *
 * Bits 15-31	Case delta: delta for case conversions.  This should be the
 *			    highest field so we can easily sign extend.
 */

static const
int groups\[\] = {"
    set line "   "
    set last [expr {[llength $groups] - 1}]
    for {set i 0} {$i <= $last} {incr i} {
	foreach {type toupper tolower totitle} [split [lindex $groups $i] ,] {}

	# Compute the case conversion type and delta

	if {$totitle != ""} {
	    if {$totitle == $toupper} {
		# subtract delta for title or upper
		set case 4
		set delta $toupper
	    } elseif {$toupper != ""} {
		# subtract delta for upper, subtract 1 for title
		set case 5
		set delta $toupper
	    } else {
		# add delta for lower, add 1 for title
		set case 3
		set delta $tolower
	    }
	} elseif {$toupper != ""} {
	    # subtract delta for upper, add delta for lower
	    set case 6
	    set delta $toupper
	} elseif {$tolower != ""} {
	    # add delta for lower
	    set case 2
	    set delta $tolower
	} else {
	    # noop
	    set case 0
	    set delta 0
	}

	set val [expr {($delta << 15) | ($case << 5) | $type}]

	append line " "
	append line [format "%d" $val]
	if {$i != $last} {
	    append line ","
	}
	if {[string length $line] > 65} {
	    puts $f $line
	    set line "   "
	}
    }
    puts $f $line
    puts $f "};

#if 0
/*
 * The following constants are used to determine the category of a
 * Unicode character.
 */

#define UNICODE_CATEGORY_MASK 0X1F

enum {
    UNASSIGNED,
    UPPERCASE_LETTER,
    LOWERCASE_LETTER,
    TITLECASE_LETTER,
    MODIFIER_LETTER,
    OTHER_LETTER,
    NON_SPACING_MARK,
    ENCLOSING_MARK,
    COMBINING_SPACING_MARK,
    DECIMAL_DIGIT_NUMBER,
    LETTER_NUMBER,
    OTHER_NUMBER,
    SPACE_SEPARATOR,
    LINE_SEPARATOR,
    PARAGRAPH_SEPARATOR,
    CONTROL,
    FORMAT,
    PRIVATE_USE,
    SURROGATE,
    CONNECTOR_PUNCTUATION,
    DASH_PUNCTUATION,
    OPEN_PUNCTUATION,
    CLOSE_PUNCTUATION,
    INITIAL_QUOTE_PUNCTUATION,
    FINAL_QUOTE_PUNCTUATION,
    OTHER_PUNCTUATION,
    MATH_SYMBOL,
    CURRENCY_SYMBOL,
    MODIFIER_SYMBOL,
    OTHER_SYMBOL
};

/*
 * The following macros extract the fields of the character info.  The
 * GetDelta() macro is complicated because we can't rely on the C compiler
 * to do sign extension on right shifts.
 */

#define GetCaseType(info) (((info) & 0xE0) >> 5)
#define GetCategory(info) ((info) & 0x1F)
#define GetDelta(info) (((info) > 0) ? ((info) >> 15) : (~(~((info)) >> 15)))
#endif

/** Extract information about a Unicode character.
 *
 *  This function extracts the information about a character from the
 *  Unicode character tables.
 */
int
Xapian::Unicode::Internal::get_character_info(unsigned ch)
{
    Assert(ch < 0x110000);
    return (groups\[groupMap\[(pageMap\[((int)(ch)) >> OFFSET_BITS\] << OFFSET_BITS) | ((ch) & ((1 << OFFSET_BITS)-1))\]\]);
}"

    close $f
}

uni::main

return
