#!/usr/local/bin/perl

#-------------------------------------------------------------------
# some stuff about cvssearch
# This produces a html query interface for cvssearch
#
# Structure:
# 1. Calls $search - interface to omsee - for query matches
# 2. grep source files for query words
# 3. sorts and rank query matches as per file
# 4. finds context matched by calling on $query
# 5. Displays results, linking them to QueryFile.cgi
#
# Author: Annie - anniec@cse.unsw.edu.au
# Date: Feb 16 2001
#-------------------------------------------------------------------

use CGI ':all';

#-------------------
# path variables
#-------------------
$CVSDATA = $ENV{"CVSDATA"}; # path where database content file is stored
$search = "$CVSDATA/bin/cvssearch";
$num_matches = 500;
$query_script = "$CVSDATA/bin/db_query_script";
$database_script = "$CVSDATA/bin/cvsupdatedb.pl -f";
$data_path = "$CVSDATA/database/";
$all = ".";
$src = "/src";

# control character separator
$querysep = chr(01);
$revsep = chr(02);
$comsep = chr(03);

# types
#$ftype = "f:"; #file type
#$ctype = "c:"; # comment type
#$gtype = "g:"; # grep type

#-------------
# start html
#-------------

print "Content-type:text/html\n\n";
print "<html>\n";
print "<header>\n";

#---------------
# style sheet
#---------------
print <<_STYLE_;
<STYLE TYPE=text/css>
<!--
body {background-color:white}
h1 {color:#0066cc; font-size:x-large}
table {border-width:0;width:100%;padding:2}
td {background-color:#3366cc;white-space:nowrap}
error (color:#ee0000; font-weight:bold}
.stats {color:white;font-family:arail,sans-serif;font-size:-1}
.blue {color:#0066cc}
.orange {color:orange;}
-->
</STYLE>
</header>
_STYLE_

#----------------------------------------
# first pass on input
# to get query for printing out the form
#----------------------------------------
if(param()){
	$query = param("query");
}

#--------------
# print form
#--------------
print <<_HTML_;
<body>
<h1>CVSSearch</h1>
<p>
<form action=$0>
<b>Enter keyword(s) to search for: </b><input type=text size=50 name=query value="$query">
<input type=submit></form>
_HTML_

#--------------
# print survey
#--------------
print <<_HTML_;
<p>
<b class=orange>Please fill out our short <a href=http://www.pollcat.com/tzk3nkgszq_a target=_blank>survey</a>. Your feedback is essential in improving CVSSearch.</font></b>
_HTML_


#--------------
# parse input
#--------------

if($query && ($query ne "")){		
		
	#--------------------------------------
	# find projects & query omsee database
	#--------------------------------------
	
	@temp = split /in:/, $query;
	$querywords = $temp[0..$#temp-2];
	@projects = split /;| /, $temp[$#temp];
	if($#projects==0){# project not specified, search in all database
		@matches = `$search $all $num_matches $query`;
		push @rawprojects, $all;
	}else{
		# find databases to perform search in
		foreach (@projects){
			#s/\//_/g;
			@cur = `$database_script $_`;
			push(@rawprojects, @cur);
		}
		if($#rawprojects!=0){
			@matches = `echo @rawprojects | $search $num_matches $querywords`;
		}else{#no project matched
			 &error("The project you specified was not found!");
		}
	}
	
	$stemquery = shift @matches;
	
	#----------------------------------
	# find grep matches
	#----------------------------------
	
	#turn query words into "or" grep format
	$grepquery = $stemquery;
	chomp($grepquery);
	$grepquery =~s/ /|/g;
	
	foreach (@rawprojects){
		push @grepmatches, &grepMatch($_);
	}

	#-----------------------------------
	# parse matches
	#-----------------------------------
	
	if($#matches==0&&$#grepmatches==0){#no match found
		&error("Sorry your query did not match any files!");
	}
	
	foreach (@matches){
		# each entry of the form
		# weight line:fileid database:revision1 revision2
		chomp;
		($wl,$file,$revs) = split /:/, $_;
		($id,$db) = split / /, $file;
		
		# keep track of distinct revision per file
		# used later when displaying cvs comments summary
		@revs = split / /, $revs;
		%revs = &insertHash(\%revs, $file, \@revs);
		
		# group files by database, used for querying info
		%dataTid = &insertArray(\%data_id, $db, $id);
		
		# group lines by file, used for calculating weight per file
		($line, $weight) = split / /, $wl;
		%filelines = &insertLine(\%filelines, $file, $line, $weight);
		
		# stores revision for each line
		$linerevs{"$file $line"} = $revs;
		
		# keep count of number of cvs matches
		$cvscount{$file} += 1;
	}
	
	#--------------------------------
	# query for match file info
	#--------------------------------
	
	while (($db, $idarray)=each %data_id){
		foreach (@$idarray){
			$id = $_;
			$querystr = "$query_script $database -f $id";
			%tmprev = &hashVal(\%revs, "$id $db");
			foreach (keys %tmprev){
				$querystr .= " -c $id $_";
			}
			$curid = `$querystr`;
			@temp = split /$querysep/g, $curid;
			$tempfilename = shift @temp;
			chomp ($tempfilename);
			
			#map id to filename
			$idTname{"$id $db"} = $tempfilename;
			$nameTid{$tempfilename} = "$id $db";
			
			$temprevs = "";
			#save comments
			%temp = &hashVal(\%revs, "$id $db");
			foreach (keys %temp){
				$temprevs .= $_.":".(shift @temp)."\n";
			}
			$idTcomment{"$id $db"} = $temprevs;
		}
	}
	

	#----------------------------------
	# parse grep matches
	#----------------------------------
	
	foreach (@grepmatches){
		@temp = split /:/, $_;
		$filename = &tofilename(shift @temp);
		$line = shift @temp;
		$source = join ':', @temp;
		$id = $nameTid{$filename};
		if($id){
			$filename = $id;
		}
		
		$idTcomment{$filename} .= "grep:".$line.":".$source."\n";

		# keep count of number of grep matches
		$grepcount{$filename} +=1;
		$greplines{"$filename $line"} = "g"; #can be anything
		#for caculating weight, give wight of 50 to grep matches
		%filelines = &insertLine(\%filelines, $filename, $line, 50);
		
	}
	
	#-----------------------------
	# calculate & sort matches
	#-----------------------------
	
	#calculate weight for each file
	%fileweight = &calcWeight(\%filelines);
	
	#-------------------------------------------------
	# display result
	# first need to dump information on a file
	# need:
	# 1. fileid
	# 2. filename
	# 3. revs: revs ...
	# 4. line weight grep revs ...
	#
	# info needed to display result:
	# 1. filename of where this information is dumped
	# 2. filename or database id
	# 3. number of matches (both, grep, cvs)
	# 4. cvs comments
	# 5. grep matches
	#-------------------------------------------------
	
	# open file for storing info
	$storefile = &validFilename($query);
	open (STORE, "> cache/$storefile") or die "can't open $storefile";
	
	# reverse sort on value
	foreach (sort {$fileweight{$b} <=> $fileweight{$a}} keys %fileweight){
		$fileid = $_;
		#------------------
		# store result
		#------------------
		if(grep / /, $_){ # using database fileID
			print STORE $_."\n"; # database id
			$filename = $idTname{$_};
			print STORE $filename."\n"; #filename
			%tmprev = &hashVal(\%revs, $_);
			@revs = keys %tmprev;
			print STORE "@revs"."\n"; #revs
				
		}else{ #using filename
			$filename = $fileid;
			print STORE $_."\n"; # filename
		}
		#store line information
		$hashptr = $filelines{$fileid};
		%lw = %$hashptr;
		while (($line, $weight)=each %lw){
			print STORE "$line $weight ";
			if($greplines{"$fileid $line"}){
				print STORE "g "; # tag grep match
			}
			print STORE $linerevs{"$file $line"}."\n"; # revisions
		}
		print STORE $querysep;
		
		#----------------------------
		# display result
		#----------------------------
		&stats;
		
		&formatFile($fileid, $filename, scalar(keys %lw), $cvscount{$fileid}, $grepcount{$fileid}); # print filename
		&formatComments($idTcomment{$fileid}); # print matched results
		

	}

	close STORE;

}

&printTips;

#--------------------------
# sub functions
#--------------------------

#!!!!!!!!!!!!!!!!!!!!
# HTML FORMATTING
#!!!!!!!!!!!!!!!!!!!!

sub stats{
	$num = scalar(keys %fileweight);
	print "<table><tr class=stats><td>";
	print "Searched CVS for <b>$stemquery</b>. &nbsp;";
	print "</td><td align=right>Matched <b>$num</b> files.";
	print "</td></tr></table>\n";
}

#-------------------
# formats html file
#-------------------
sub formatFile{
	my ($data, $filename, $totalmatch, $cvsm, $grepm) = @_;
	print "<p><a href=$data$storefile>$filename</a> matched lines: $totalmatch [$cvsm cvs matches] [$grepm grep matches]\n";
}

#----------------------------------------------------
# given revision and its comments, format comments
# by making words matched with query words bold.
#----------------------------------------------------
sub formatComments{
	my ($comments) @_;
	@sep = split /:/, $comments;
	$head = shift @sep;
	$rest = join ':', @sep;
	my $output = "<br><b class=blue>$head: <b>";
	@contains =grep s/($grepquery)/<b>\1<\/b>/ig, $rest;
	foreach (@contains){
		$output .= "..$_..";
	}
	return my $output;
}
#sub formatComments{
#	my ($rev, $comments) = @_;
	
#	my $output = "<br><b class=blue>$rev: <b>";
#	my @contains = grep s/($grepquery)/<b>\1<\/b>/ig, $comments;
	
#	foreach (@contains){
#		$output .= "..$_..";
#	}
#	return $output;
#}

#--------------
# tips
#--------------
sub printTips{
	print "<p>\n";
	print "<b class=blue>Tips</font></b>\n";
	print "<ul>\n";
	print "<li>use <tt class=orange>in:</tt> at the end of keywords to select package to seach in. For example, \n";
	print "<br><tt class=orange>menu in:kdebase/konqueror;kdepime/korganizer</tt> \n";
	print "<br>searches for menu under kdebase/konqueror and kdepim/korganizer, default searches for keywords under all packages.\n";
	print "<li>Keywords are not case-sensitive and stemmed. (e.g. searching for 'fishes' will match 'FISH', 'fishes', 'fishing'...)\n";
	print "<li>Results are ranked with the objective of matching as large a fraction of the keywords as possible.  \n";
	print "The top 500 lines are returned.\n";
	print "</ul>\n";
	
	print "</body>";
	print "</html>";
}

#------------------
# display errors
#------------------
sub error{
	($mesg) = @_;
	print "<p><error>$mesg</error>";
	&printTips;
	exit(0);
}

#-------------------------------------------
# encode given string into url
#-------------------------------------------
sub encode{
	$string = @_;
	$string =~ s/([^a-zA-Z0-9])/'%'.unpack("H*",$1)/eg;
	return my $string;
}

#!!!!!!!!!!!!!!!!!!
# Grep functions
#!!!!!!!!!!!!!!!!!!

#------------------------------------
# grep source file for query string
#------------------------------------
sub grepMatch{
	my ($path) = @_;
	my $path = $data_path.$path.$src;
	$findfiles = `find $path -iregex '.*.java\\|.*.pl\\|.*.cpp\\|.*.c++\\|.*.cc\\|.*.h\\|.*.c\\|.*.rc\\|.*.sh'`;
	$findfiles =~ s/\n/ /g;
	my @curmatch = `grep -I -i -n -H '$grepquery' $findfiles`;
	return @curmatch;
}

#!!!!!!!!!!!!!!!!!!!!
# Calculation
#!!!!!!!!!!!!!!!!!!!!

#-----------------------------------------------
# calculates the weight of a file by:
# sum(i,j)(wi*wj/(i-j)^2)
#-----------------------------------------------
sub calcWeight{
	my ($hashptr) = @_;
	my %hash = %$hashptr;
	my %fileweight;
		
	while (($key,$val)=each %hash) {
		my $sumweight = 0;	
		while (($line1, $weight1)=each %$val){
			while (($line2, $weight2)=each %$val){
				if($line1 != $line2){					
					$sumweight+=($weight1*$weight2)/(($line1-$line2)*($line1-$line2));
				}
			}
		}		
		$fileweight{$key} = $sumweight;
	}
	return %fileweight;
}

#!!!!!!!!!!!!!!!!!!!!!!!
# Hashtable functions
#!!!!!!!!!!!!!!!!!!!!!!!

#--------------------------------------------
# Given a hashtable with array as its value
# push new value into array if entry exists
# Usage:
# %hash = &insertArray(\%hash, $key, $value);
#--------------------------------------------
sub insertArray{
	my ($hashptr,$key,$value) = @_;
	my %hash = %$hashptr;
	if($hash{$key}){
		my $cur = $hash{$key};
		push @$cur, $value;
	}else{
		push my @cur, $value;
		$hash{$key} = \@cur;
	}
	return %hash;
}

#--------------------------------------------
# Given a hashtable with hash as its value
# insert value in given array into hash
# Usage:
# %hash = &insertHash(\%hash, $key, \@array);
#--------------------------------------------
sub insertHash{
	my ($hashptr,$key, $value) = @_;
	my %hash = %$hashptr;
	my @rev = @$value;
	if($hash{$key}){
		my $cur = $hash{$key};
		%hashrev = %$cur;
		foreach (@rev){
			$hashrev{$_} = 1;
		}
	}else{
		foreach (@rev){
			$hashrev{$_} = 1;
		}
		$hash{$key} = \%hashrev;
	}
	return %hash;	
}

#--------------------------------------------
# Given a hashtable with another hashtable
# as value, insert $line $value into that
# subhashtable if line exists add value onto
# the existing
# Usage:
# %hash = &insertLine(\%hash, $key, $line, $weight);
#--------------------------------------------
sub insertLine{
	my ($hashptr,$key, $line, $weight) = @_;
	my %hash = %$hashptr;
	
	if($hash{$key}){
		my $cur = $hash{$key};
		my %hashlw = %$cur;
		$hashlw{$line} += $weight;
	}else{
		$hashlw{$line} = $weight;
		$hash{$key} = \%hashlw;
	}
	return %hash;	
}


#---------------------------------------------
# Given a hashtable with array as its value
# returns the array the key maps to
# Usage:
# @value = &arrayVal(\%hash, $key);
#---------------------------------------------

sub arrayVal{
	my ($hashptr, $key) = @_;
	my %hash = %$hashptr;
	$value = $hash{$key};
	return @$value;
}

sub hashVal{
	my ($hashptr, $key) = @_;
	my %hash = %$hashptr;
	$value = $hash{$key};
	#return sort {(split /\./, $b)[1] <=> (split /\./, $a)[1]} keys %$value;
	return %$value;
}

#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# Misc Functions
#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#---------------------------------------------
# Convert $CVSDATA/database/xxx/src/filename
# to filename
#---------------------------------------------

sub tofilename{
	$tmpfilename = @_;
	$tmpfilename =~ s/^$CVSDATA\/database\/\w*\/src\///;
	return my $tmpfilename;
}

sub toFullpath{
	my ($database,$filename) = @_;
	my $fullpath = "$CVSDATA/database/$database/src/$filename";
	return my $fullpath;
}

#--------------------------------------------
# make the given string into valide filename
# by replacing non characters with ord(c)
#--------------------------------------------

sub validFilename{
	$string = @_;
	$string =~ s/(\W|\s)/@{[ord($1)]}/g;
	return my $string;
}

#!!!!!!!!!!!!!!!!!!!!!!!!!!
# debugging functions
#!!!!!!!!!!!!!!!!!!!!!!!!!!

sub printarrayVal{
	print "-----------filelines--------------\n";
	my ($hashptr) = @_;
	my %hash = %$hashptr;
	while (($key,$val)=each %hash) {
		print $key."-";
		$,=':';
		print @$val;
		print "\n";
	}
}

sub printhashVal{
	print "---------revisions----------\n";
	my ($hashptr) = @_;
	my %hash = %$hashptr;
	while (($key,$val)=each %hash) {
		print $key."-";
		print sort keys %$val;
		print "\n";
	}
}
