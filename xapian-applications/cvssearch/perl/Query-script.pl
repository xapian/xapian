#-------------------------------------------------------------------
# some stuff about cvssearch
# This produces a html query interface for cvssearch
#
# Structure:
# 1. Calls $cvssearch - interface to omsee - for query matches
# 2. grep source files for query words
# 3. sorts and rank query matches as per file
# 4. finds context matched by calling on $query
# 5. Displays results, linking them to QueryFile.cgi
#
# Author: Annie - anniec@cse.unsw.edu.au
# Date: Feb 16 2001
#-------------------------------------------------------------------


use CGI ':all';
use cvssearch;
use Entities ();

#-------------------
# path variables
#-------------------
$CVSDATA = &cvssearch::get_cvsdata(); # path where database content file is stored
$cvssearch = "./cvssearch";
$num_matches = 1500;
$cvsquery = "./cvsquerydb";
$cvsupdate = "./cvsupdatedb";
$CVSROOTS = "$CVSDATA/CVSROOTS";
$all = "All";
$queryfile = "Queryfile.cgi";
$cache = "./cache"; #where dump is written to

#---------------------------------------------
# global mappigns defind in the script
#---------------------------------------------
# file = root database fileid
# filename = full path
# %dirMAProot - map root directory to cvsroot
# %rootMAPproj - map root directory to @projects specified in the search
# %idMAPfile - map id to filename, where id is of the form root1 kdepim_konq 80
# %idMAPrelPath - map id to relative path name
# %fileMAPid - map filename to id
# %fileMAPrev - map file to %revisions 
# %fileMAPcomment - map file to revision comments (comments separated by ctrlA, in the order of (keys %($fileMAPrev{file})))
# %fileMAPgrep - map file to @(lines of source code) matched by grep
# %databaseMAPid - map database (root db) to %files (id)s
# %fileMAPweight -  map file to %(line->weight)
# %lineMAPrevs - map line (file line) to revisions (grep rev1 rev2)
# %fileMAPcvscount - map file to the number of cvs matches
# %fileMAPgrepcount - map file to the number of grep matches


# control character separator
$ctrlA = chr(01);
$ctrlB = chr(02);
$ctrlC = chr(03);

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
.red {color:red}
.lightgreen {color:#49C96B}
.lightcvs {color:#00ccff}
.blue {color:#0066cc}
.orange {color:#FF7A12}
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
	$root = param("root");
}

#--------------
# print form
#--------------
print <<_HTML_;
<body>
<h1>CVSSearch</h1>
<p>
<form action=./Query.cgi>
<b>Enter keyword(s) to search for: </b><input type=text size=50 name=query value="$query">
_HTML_

# print roots
print "<p><b>Select Repository: </b><select name=root>\n";
print "<OPTION>$all</OPTION>\n";

open ROOTS, "< $CVSROOTS" or die "can't open cvsroots";
while (<ROOTS>) {
		($curroot, $curdir) = split /\s/, $_;
		$dirMAProot{$curdir} = $curroot;
		print "<OPTION";
		if ($root eq $curdir){
			print " selected";
		}
		print " value=$curdir>$curroot</OPTION>\n";
}
close ROOTS;

print <<_HTML_;
</select>
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
	# find projects
	#--------------------------------------
	#print "<p>$CVSDATA";#DEBUG
	@temp = split /in:/, $query;
	if($#temp>0){
		$tempp = pop @temp;
	}else{
		$tempp = "";
	}
	$querywords = join "in:", @temp;
#	print $query."<br>";#DEBUG
#	print "@temp<br>";#DEBUG
#	print $querywords."<br>";#DEBUG
	@rawproj = split /;| /, $tempp;
	#print @rawproj;#DEBUG
	$rootproj = ''; #root:project pairs to give to cvssearch
	
	if($root eq $all){# selected all repositories
	
		# go through each root
		foreach (keys %dirMAProot){
			$curroot = $_;
			if($#rawproj<0){# search through all project under the root
				@curproj = `$cvsupdate $curroot -f .`;
				#print "@curproj";
				#print "debug:".@curproj; # DEBUG
				#print "all project";#debug;
				foreach (@curproj){
					chomp;
					#store project for grep later
					%rootMAPproj=&insertArray(\%rootMAPproj, $curroot, $_);
#					print "PROJECTS:";#debug
#					print $_;#debug
#					print ";";#debug
					#for input of cvssearch
					$rootproj .= "$curroot/db/$_.om ";
				}
			}else{
				#print "raw projct";#debug;
				foreach(@rawproj){
					s/\//_/g;
					@curproj = `$cvsupdate $curroot -f $_`;
					
					foreach (@curproj){
						chomp;
						#store project for grep later
						%rootMAPproj=&insertArray(\%rootMAPproj, $curroot, $_);
						#for input of cvssearch
						$rootproj .= "$curroot/db/$_.om ";
					}						
				}
			}
		}
	
	}else{#selected one root

		$curroot = $root;
		if($#rawproj<0){# search through all project under the root
			@curproj = `$cvsupdate $curroot -f .`;
			
			foreach (@curproj){
				chomp;
				#store project for grep later
				%rootMAPproj=&insertArray(\%rootMAPproj, $curroot, $_);
				#for input of cvssearch
				$rootproj .= "$curroot/db/$_.om ";
			}
		}else{
			foreach(@rawproj){
				s/\//_/g;
				@curproj = `$cvsupdate $curroot -f $_`;
				
				foreach (@curproj){
					chomp;
					#store project for grep later
					%rootMAPproj=&insertArray(\%rootMAPproj, $curroot, $_);
					#for input of cvssearch
					$rootproj .= "$curroot/db/$_.om ";
				}						
			}
		}
	}
	
	#---------------
	# query omsee
	#---------------
	
	if($rootproj ne ''){
		#print "<p>echo $rootproj | $cvssearch $num_matches $querywords";
        @matches = `echo $rootproj | $cvssearch $num_matches $querywords`;
	}else{#no project matched
		 &error("The project you specified was not found!");
	}
	
	#print "<p>finished omsee";#DEBUG
	#print "<p>omsee result: <br><pre>";#DEBUG
	#print @matches;#DEBUG
	
	$stemquery = shift @matches;
	chomp($stemquery);
	#print "<p>stemquery: $stemquery\n";#DEBUG
	
	#----------------------------------
	# find grep matches
	#----------------------------------
	
	#turn query words into "or" grep format
	$grepquery = $stemquery;
	chomp($grepquery);
	$grepquery =~s/ /|/g;
	
	#---------------------------------------------
	#find files to grep and insert in fileMAPid
	#---------------------------------------------

	while (($curroot, $curproject) = each %rootMAPproj){
		foreach(@$curproject){
			$curprojname = $_;
			#print "PROJECT: $curroot $_";#debug
			# look at offset file for files in search db
			#print "<p>$CVSDATA/$curroot/db/$curprojname.offset";
			@file = `cat $CVSDATA/$curroot/db/$curprojname.offset`;
			$i = 1; #index for file id
			foreach (@file){
				($curfile) = split /\s/, $_; #get file name
				$curprojname =~ s/\//_/g;
				$curid = "$curroot $curprojname $i";
				$idMAPrelPath{$curid} = $curfile;
				$curfile = "$CVSDATA/$curroot/src/$curfile";
				$idMAPfile{$curid} = $curfile;
				$fileMAPid{$curfile} = $curid;
				$i++;
			}
		}
	}
	
	
	#print "<P>finished going through offset to find files\n";#DEBUG
	
	#----------------
	# grep files
	#----------------
	
	$files = join ' ', (values idMAPfile);
	#print "<p>grep -I -i -n -H '$grepquery' $files | head -$num_matches";
	@grepmatches = `grep -I -i -n -H '$grepquery' $files | head -$num_matches`;

	#print "<p>finished grep\n";#DEBUG
	#print "<p>grep result: @grepmatches";#debug

	#-----------------------------------
	# parse matches
	#-----------------------------------
	
	if($#matches<0&&$#grepmatches<0){#no match found
		&error("Sorry your query did not match any files!");
	}
	
	#---------------------
	# parse cvs matches
	#---------------------
	
	foreach (@matches){
		# each entry of the form
		# weight line:root database fileid:revision1 revision2
		#print $_;#debug
		chomp;
		($wl,$file,$revs) = split /:/, $_;
		($curroot, $db, $id) = split /\s/, $file;
		
		# keep track of distinct revision per file
		# used later when displaying cvs comments summary
		@revs = split /\s/, $revs;
		%fileMAPrev = &insertHash(\%fileMAPrev, $file, \@revs);
		
		# group files by database, used for querying info
		%databaseMAPid = &insertHashOne(\%databaseMAPid, "$curroot $db", $id);
		
		# group lines by file, used for calculating weight per file
		($weight, $line) = split /\s/, $wl;
		
		#print "insert $file, $line:$weight\n";#debug
		
		%fileMAPweight = &insertLine(\%fileMAPweight, $file, $line, $weight);
		
		# stores revision for each line
		$lineMAPrevs{"$file $line"} = $revs;
		
		# keep count of number of cvs matches
		$fileMAPcvscount{$file} += 1;
		
	}
	
	#print "<p>finished parsing cvs\n";#DEBUG
	#--------------------------------
	# cvsquery for match file info
	#--------------------------------
	
	# for each database
	while (($db, $idarray)=each %databaseMAPid){
		$querystr = "$cvsquery $db";
		#go through each file
		foreach (keys %$idarray){
			$id = $_;
			%tmprev = &hashVal(\%fileMAPrev, "$db $id");
			foreach (keys %tmprev){
				$j++;
				$querystr .= " -c $id $_";
			}
		}
		#print "<p> $querystr";#debug
		$curid = `$querystr`;
		#print "<br> $curid\n";#debug
		@revs = split /$ctrlA/, $curid; # split revisions
		#print "<br> splitted into $#revs revisions:$revs[$#revs-1]";#debug
		$i=0; # for where in @revs does the revision of the current file starts
		#save comments for each file
		
		foreach (keys %$idarray){
			$id = $_;
			#work out number of revisions
			%tmprev = &hashVal(\%fileMAPrev, "$db $id");
			$numrev = scalar(keys %tmprev);
			$bg = $i;
			$ed = $bg+$numrev-1;
			#print "<br>i=$i, numrev=$numrev, $bg..$ed";#debug
			@filerev = @revs[$bg..$ed]; #revision for that file
			#print " result:$#filerev";#debug
			$i += $numrev;
			$fileMAPcomment{"$db $id"} = join $ctrlA, @filerev;
			
		}
	}

	#print "<p>finished cvsquery\n";#DEBUG

	#----------------------------------
	# parse grep matches
	#----------------------------------
	
	foreach (@grepmatches){
		@temp = split /:/, $_;
		$curfile = shift @temp;
		$line = shift @temp;
		$source = join ':', @temp;
		
		$curid = $fileMAPid{$curfile};
		
		# save source code
		%fileMAPgrep =  &insertArray(\%fileMAPgrep, $curid, "$line: $source");

		# keep count of number of grep matches
		$fileMAPgrepcount{$curid} +=1;
		
		#for dump file
		$lineMAPrevs{"$curid $line"} = "grep ".$lineMAPrevs{"$curid $line"};
		
		#for caculating weight, give wight of 50 to grep matches
		%fileMAPweight = &insertLine(\%fileMAPweight, $curid, $line, 50);
		
	}

	#print "<p>finished parsing grep matches\n";#DEBUG
	#-----------------------------
	# calculate & sort matches
	#-----------------------------
	
	#calculate weight for each file
	%fileweight = &calcWeight(\%fileMAPweight);
	
	#print "<p>finished calculating weight\n";#DEBUG
	
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
	
	&stats;
	
	# open file for storing info
	$storefile = &validFilename($query);
	#print "$cache/$storefile\n";#DEBUG
	if (!(-d "$dir")) {
		system("mkdir $cache");
	}
	open (STORE, "> $cache/$storefile") or die "can't open $storefile";
	
	#print query info here
	#print "query:$query stem:$stemquery grep:$grepquery\n</pre>";
	print STORE "$query\n$stemquery\n$grepquery\n$ctrlA";	
	print "<FONT face=arial,sans-serif>";
	# reverse sort on value
	foreach (sort {$fileweight{$b} <=> $fileweight{$a}} keys %fileweight){
		$curid = $_;
		#print $curid; #debug
		#------------------
		# store result
		#------------------
		
		print STORE $curid."\n"; # database id
		$filename = $idMAPfile{$curid};
		print STORE $filename."\n"; #filename
		%tmprev = &hashVal(\%fileMAPrev, $curid);
		@revs = keys %tmprev;
		print STORE "@revs"."\n"; #revs
		
		#store line information
		$hashptr = $fileMAPweight{$curid};
		%lw = %$hashptr;
		while (($line, $weight)=each %lw){
			print STORE "$line $weight ";
			print STORE $lineMAPrevs{"$curid $line"}."\n"; # revisions
		}
		print STORE $ctrlA;
		
		#----------------------------
		# display result
		#----------------------------
		
		$displayfile = &displayFile($curid);
		$cvscount = $fileMAPcvscount{$curid};
		$grepcount = $fileMAPgrepcount{$curid};
		$totalcount = scalar(keys %lw);
		
		#print filename and link
		$encodedid = &cvssearch::encode($curid);
		$encodename = &cvssearch::encode($displayfile);
		print "<p><a href=$queryfile?id=$encodedid&dump=$storefile&displayname=$encodename>$displayfile</a>";
		print "<font size=-1>";
		print " matches $totalcount lines: ";
		print "<b>";
		if($cvscount){
			print "<font class=lightcvs>[$cvscount cvs]</font>";
		}
		if($grepcount){
			print "<font class=lightgreen>[$grepcount grep]</font>";
		}
		print "</b>";
		
		#print comments
		@comments = split /$ctrlA/, $fileMAPcomment{$curid};
		$revsptr = $fileMAPrev{$curid};
		@revs = keys %$revsptr;
		#print "<br>@comments<br>";#debug
		#print "@revs<br>";#debug
		#print "<pre>";
		for ($i=0;$i<=$#revs;$i++){
			$tmpcomment = $comments[$i];
			$tmpcomment = Entities::encode_entities($tmpcomment);
			@highlight = &highlightquery($tmpcomment);
			if(@highlight){
				print "<br><b class=lightcvs>$revs[$i]:</b>";
				foreach (@highlight){
					s/\n//g;
					print "..$_..";
				}
			}
		}
		
#		#print grep results
#		$grepptr = $fileMAPgrep{$curid};
#		foreach (@$grepptr){
#			@sep = split /:/, $_;
#			$curline = shift @sep;
#			$cursource = join ':', @sep;
#			$cursource = Entities::encode_entities($cursource);
#			#print $cursource;#debug
#			@highlight = &highlightquery($cursource);
#			print "<br><b class=lightgreen>$curline:</b>";
#			foreach (@highlight){
#				print "$_";
#			}
#		}
		#print "</pre>";
		print "</font>";
	}
	print "</font>";
	close STORE;

}

&printTips;

#--------------------------
# sub functions
#--------------------------

#!!!!!!!!!!!!!!!!!!!!
# HTML FORMATTING
#!!!!!!!!!!!!!!!!!!!!

#-----------------------------------
# stats prints stemmed query words
# projects searched in and
# number of files matched
#-----------------------------------

sub stats{
	$num = scalar(keys %fileweight);
print <<_HTML_;
<p>
<TABLE cellSpacing=0 cellPadding=2 width="100%" border=0>
<TBODY>
<TR>
<TD noWrap bgColor=#3366cc><FONT face=arial,sans-serif color=white 
size=-1>Searched CVS for <b>$stemquery</b> in<b>
_HTML_

	while(($curroot, $curproj) = each %rootMAPproj){
		#repository name
		$currep = $dirMAProot{$curroot};
		print " $currep:";
		foreach (@$curproj){
			s/_/\//;
			#print project
			print "$_;";
		}
	}

print <<_HTML_;
</b>&nbsp; </FONT></TD>
<TD noWrap align=right bgColor=#3366cc><FONT face=arial,sans-serif color=white 
size=-1>Matched <B>$num</B> files.</FONT></TD></TR></TBODY></TABLE>
_HTML_
}

#----------------------------
# display file name from id
# returning rep:project/src
#----------------------------
sub displayFile{
	my ($id) = @_;
	my $relpath = $idMAPrelPath{$id};
	my ($root) = split /\s/, $id;
	my $rep = $dirMAProot{$root};
	return "$rep:$relpath";
}

#-----------------------------------
# highlightquery
# return line matched by query words
# make words matched by query bold
#-----------------------------------
sub highlightquery{
	my ($words) = @_;
	@contains = grep s/($grepquery)/<b>\1<\/b>/ig, $words;
	return @contains;
}

##-------------------
## formats html file
##-------------------
#sub formatFile{
#	my ($data, $filename, $totalmatch, $cvsm, $grepm) = @_;
#	print "<p><a href=$data$storefile>$filename</a> matched lines: $totalmatch [$cvsm cvs matches] [$grepm grep matches]\n";
#}
#
##----------------------------------------------------
## given revision and its comments, format comments
## by making words matched with query words bold.
##----------------------------------------------------
#sub formatComments{
#	my ($comments)= @_;
#	@sep = split /:/, $comments;
#	$head = shift @sep;
#	$rest = join ':', @sep;
#	my $output = "<br><b class=blue>$head: <b>";
#	@contains =grep s/($grepquery)/<b>\1<\/b>/ig, $rest;
#	foreach (@contains){
#		$output .= "..$_..";
#	}
#	return my $output;
#}
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
	print "The top 1500 lines are returned.\n";
	print "</ul>\n";
	
	print "</body>";
	print "</html>";
}

#------------------
# display errors
#------------------
sub error{
	($mesg) = @_;
	&stats;
	print "<p><b class=red>$mesg</b>";
	&printTips;
	exit(0);
}


#!!!!!!!!!!!!!!!!!!
# Grep functions
#!!!!!!!!!!!!!!!!!!

##------------------------------------
## grep source file for query string
##------------------------------------
#sub grepMatch{
#	my ($path) = @_;
#	my $path = $data_path.$path.$src;
#	$findfiles = `find $path -iregex '.*.java\\|.*.pl\\|.*.cpp\\|.*.c++\\|.*.cc\\|.*.h\\|.*.c\\|.*.rc\\|.*.sh'`;
#	$findfiles =~ s/\n/ /g;
#	my @curmatch = `grep -I -i -n -H '$grepquery' $findfiles`;
#	return @curmatch;
#}

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
		
	while (my ($key,$val)=each %hash) {
		my %tmp1 = %$val;
		my %tmp2 = %$val;
#		print "$key:";#debug
#		print %tmp1;#debug
#		print "\n";#debug
		my $sumweight = 0;	
		while (my ($line1, $weight1)=each %tmp1){
			while (my ($line2, $weight2)=each %tmp2){
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
		my %hashrev = %$cur;
		foreach (@rev){
			$hashrev{$_} = 1;
		}
		$hash{$key} = \%hashrev;
	}else{
		my %hashrev = ();
		foreach (@rev){
			$hashrev{$_} = 1;
		}
		$hash{$key} = \%hashrev;
	}
	return %hash;	
}

#--------------------------------------------
# Given a hashtable with hash as its value
# insert value in given value into hash
# Usage:
# %hash = &insertHash(\%hash, $key, $value);
#--------------------------------------------
sub insertHashOne{
	my ($hashptr,$key, $value) = @_;
	my %hash = %$hashptr;
	if($hash{$key}){
		my $cur = $hash{$key};
		my %hashid = %$cur;
		$hashid{$value} = $key;
		$hash{$key} = \%hashid;
	}else{		
		my %hashid = ();
		$hashid{$value} = $key;
		$hash{$key} = \%hashid;
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
		$hash{$key} = \%hashlw;
	}else{
		my %hashlw = ();
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

##---------------------------------------------
## Convert $CVSDATA/database/xxx/src/filename
## to filename
##---------------------------------------------
#
#sub tofilename{
#	$tmpfilename = @_;
#	$tmpfilename =~ s/^$CVSDATA\/database\/\w*\/src\///;
#	return my $tmpfilename;
#}
#
#sub toFullpath{
#	my ($database,$filename) = @_;
#	my $fullpath = "$CVSDATA/database/$database/src/$filename";
#	return my $fullpath;
#}

#--------------------------------------------
# make the given string into valide filename
# by replacing non characters with ord(c)
#--------------------------------------------

sub validFilename{
	($string) = @_;
	$string =~ s/(\W|\s)/@{[ord($1)]}/g;
	return $string;
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
		my @temp= sort keys %$val;
		print "@temp";
		print "\n";
	}
}
