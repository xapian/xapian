#-------------------------------------------------------------------
# some stuff about cvssearch
# This displays all the mining results for browsing
#
# takes 3 parameters
# 1. root
# 2. symbol (optional, specify to get all related functions)
# 3. mode [=>|<=>] (if nothing given, default is <=>)
#
# Structure:
# 1. Calls $cvsmine 
#
# Author: Annie - anniec@cse.unsw.edu.au
# Date: May 9 2001
#-------------------------------------------------------------------


use CGI ':all';
use Cvssearch;
use Entities;

#-------------------
# path variables
#-------------------
$CVSDATA = &Cvssearch::get_cvsdata(); # path where database content file is stored
$cvsmine = "./cvsminesearch";
$num_mine = "100"; # to get 100 matches
$CVSROOTS = "$CVSDATA/CVSROOTS";

$own = "./PatternBrowser.cgi"; # name of this script

#---------------------------------------------
# global mappings defined in the script
#---------------------------------------------
# UNUSED - %dirMAProot - map root directory to cvsroot
# %modeMAPcommand - map modes like => to command line parameters =\> (escape pipping)
# %modeMAPhtml - map mode to html presentation, e.g. => to =&gt;

$modeMAPcommand{"=>"} = "=\\>";
$modeMAPcommand{"<=>"} = "\\<=\\>";
##$modeMAPcommand{"<="} = "\\<=";
$modeMAPhtml{"=>"} = "=&gt;";
$modeMAPhtml{"<=>"} = "&lt;=&gt;";
##$modeMAPhtml{"<="} = "&lt;=";

#-------------
# start html
#-------------

print "Content-Type: text/html\n\n";
print "<html><head>\n";
print "<title>CVSSearch : Library Classes/Functions Search</title>\n";

#---------------
# style sheet
#---------------
Cvssearch::print_style_sheet();
print "</head>\n";
print "<body>\n";

#----------------------------------------
# get parameters
#----------------------------------------
$query = param("query");
$urlquery = $query;
$urlquery =~ s/\s/%20/g;
$symbol = param("symbol");
my $root = Cvssearch::sanitise_root(param("root"));

$mode = &Cvssearch::decode(param("mode"));

# set default mode to <=>
if (!$mode) {
    $mode = "<=>";
}
	
#-----------------------------------
# print headings and forms
#-----------------------------------
print "<table width=100%><tr><td><h1>Browse Classes &amp; Functions</h1></td>";
print "<td align=right><a href=./Query.cgi>Search Again</a></td></tr></table>\n";

# print roots form
print "<form action=$own><b>Select Repository: </b><select name=root>\n";

open ROOTS, "< $CVSROOTS";
while (<ROOTS>) {
	($curroot, $curdir) = split /\s/, $_;
	# UNUSED $dirMAProot{$curdir} = $curroot;
	  
	if(!$root){
		$root = $curdir;
	}
	print "<OPTION";
	if ($root eq $curdir){
		print " selected";
	}
	print " value=$curdir>$curroot</OPTION>\n";
}
close ROOTS;

print <<_HTML_;
</select>&nbsp;<input type=submit value="Browse"></form>
_HTML_

#-------------------------------
# search for minining on input
#-------------------------------
if($symbol||$query){
	$minequerystr = "$cvsmine $root/db/mining.om $num_mine";
	
	#---------------------------------------------
	# print heading showing symbol or query
	print "<table width=100%><tr><td><h2>";
		
	if($query){
		print "$query ";
		$minequerystr .= " $query";
	}
	
	if($query && $symbol){
		print "&amp; "
	}
	
	if($symbol){
		print "$symbol ";
		$minequerystr .= " \":$symbol\"";
	}
	
	print "<font color=#666666>[<a href=\"./QueryCommit.cgi?root=$root&symbol=$symbol&query=$urlquery\">code</a>]</font> $mode</h2></td>\n";
	
	#-------------------
	# print relations
	print "<td align=right>";
	print "<form action=$own>\n";
	print "<input type=hidden name=symbol value=$symbol></input>";
	print "<input type=hidden name=query value=\"$query\"></input>";
	print "<input type=hidden name=root value=$root></input>";
	
	print "<b>Relation:</b>&nbsp;<font class=ref>[help]</font> <select name=mode>";
	
	foreach(keys %modeMAPcommand){
		$curmode = $_;
		print "<option";
		if($curmode eq $mode){
			print " selected";
		}	
		$modeurl = &Cvssearch::encode($curmode);
		print " value=\"$modeurl\">$modeMAPhtml{$curmode}</option>\n";
	}
	
	print "</select>&nbsp;<input type=submit value=\"Switch\"></form></td></tr></table>\n";
	
	#--------------------------------
	# query cvsmine on query
	
	$minequerystr .= " $modeMAPcommand{$mode}";
	@mineResult = `$minequerystr`;
	
}else { #mine all
	#print "$cvsmine $root/db/mining.om $num_mine";
	@mineResult = `$cvsmine $root/db/mining.om $num_mine`;
		
}

#-------------------------------------------------
# parse cvsmine output and display html
#-------------------------------------------------

$gquery = shift @mineResult;
$gquery =~ s/ /|/g; 
# grep to separate functions and classesgrep 
@funcs = grep /\(\)$/, @mineResult;
$end = $#mineResult - $#funcs - 1; #functions always appears after all the classes 
@classes = @mineResult[0..$end];

# list classes/functions

print <<_HTML_;
<table cellSpacing=0 cellPadding=2 width="100%" border=0>
<tr><td colspan=2 class=s width="50%"><FONT face=arial,sans-serif size=-1>
<b>Classes</b>&nbsp; </FONT></td>
<td colspan=2 class=s width="50%"><FONT face=arial,sans-serif size=-1><b>Functions</b></td></tr>
_HTML_
		
if($#classes > $#funcs){
	$max = $#classes;
}else{
	$max = $#funcs;
}

$flag = 1;
# print classes/functions
for ($i=0; $i <= $max; $i++, $flag *=-1) {
	($rank, $curclass) = split /\s/,$classes[$i];
	($rank, $curfunc) = split /\s/,$funcs[$i];
	$curclass =~ s/$mode//;
	$curfunc=~ s/$mode//;
	
	#print class    	
	if($flag <0){
		print "<tr class=o>";
	}
	if($curclass){
		print "<td><pre>$curclass</td><td><font class=ref>[<a href=\"./QueryCommit.cgi?root=$root&symbol=$curclass%20$symbol&query=$urlquery\">code</a> / <a href=\"$own?symbol=$curclass&mode=$modeMAPhtml{$mode}&root=$root\">related</a>]</font></td>\n"; 
	}else{
 		print "<td></td><td></td>";
   	}
        	
	#print func
	if($curfunc){
        print "<td><pre>$curfunc</td><td><font class=ref>[<a href=\"./QueryCommit.cgi?root=$root&symbol=$curfunc%20$symbol&query=$urlquery\">code</a> / <a href=\"$own?symbol=$curfunc&mode=$modeMAPhtml{$mode}&root=$root\">related</a>]</font></td></tr>";        		
    }else{
    	print "<td></td><td></td>";
	}
}

print "</table></body></html>";	
	

#------------------
# display errors
#------------------
sub error{
	($mesg) = @_;
	print "<p><b class=red>$mesg</b></body></html>";
	exit(0);
}

