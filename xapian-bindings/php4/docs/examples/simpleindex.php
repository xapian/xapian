<?php
/* $Id$
 * Index each paragraph in a textfile as a document
 *
 * ----START-LICENCE----
 * Copyright 2004 James Aylett
 * Copyright 2004 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

dl('xapian.so');

define('MAX_PROB_TERM_LENGTH', 64);

function p_alnum($c)
{
    return ctype_alnum($c);
}

function p_notalnum($c)
{
    return !ctype_alnum($c);
}

function p_notplusminus($c)
{
    return $c!='+' and $c!='-';
}

function find_p($string, $start, $predicate)
{
    while ($start < strlen($string) and
	   !$predicate(substr($string, $start, 1))) {
	$start ++;
    }
    return $start;
}

if (!isset($_SERVER['argv']) or count($_SERVER['argv']) != 2) {
    print "usage: {$_SERVER['argv'][0]} <path to database>\n";
    exit;
}

$database = open_writable($_SERVER['argv'][1], DB_CREATE_OR_OPEN);
if (!$database) {
    print "Died! :-(\n";
    exit;
}
$stemmer = new_Stem("english");
$para = '';
$lines = file("php://stdin");
foreach ($lines as $line) {
    $line = rtrim($line);
    if ($line=="") {
	if ($para!="") {
	    $doc = new_Document();
	    Document_set_data($doc, $para);
	    $pos = 0;
	    /*
	     * At each point, find the next alnum character (i), then
	     * find the first non-alnum character after that (j). Find
	     * the first non-plusminus character after that (k), and if
	     * k is non-alnum (or is off the end of the para), set j=k.
	     * The term generation string is [i,j), so len = j-i
	     */
	    $i = 0;
	    $j = 0;
	    while ($i<strlen($para)) {
		$i = find_p($para, $j, 'p_alnum');
		$j = find_p($para, $i, 'p_notalnum');
		$k = find_p($para, $j, 'p_notplusminus');
		if ($k==strlen($para) or !p_alnum(substr($para, $k, 1))) {
		    $j = $k;
		}
		if ($j-$i <= MAX_PROB_TERM_LENGTH and $j>$i) {
		    $term = stem_stem_word($stemmer, substr($para, $i, $j-$i));
		    Document_add_posting($doc, $term, $pos);
		    $pos ++;
		}
		$i = $j;
	    }
	    WritableDatabase_add_document($database, $doc);
	    WritableDatabase_flush($database);
	    $para = "";
	}
    } else {
	if ($para!="") {
	    $para .= " ";
	}
	$para .= $line;
    }
}
delete_WritableDatabase($database);
?>
