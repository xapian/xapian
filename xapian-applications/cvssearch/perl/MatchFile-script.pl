#-------------------------------------------------------------------
# some stuff about cvssearch
# This produces a html with matched lines only and it's revision 
# information.
#
# Author: Annie - anniec@cse.unsw.edu.au
# Date: Feb 16 2001
#-------------------------------------------------------------------

use strict;
use CGI ':all';
use Cvssearch;
use Entities;

#-----------------
# paths
#-----------------
my $source = "./Source.cgi";
my $cvsquery = "./cvsquerydb";
my $passparam = "";

#---------------------------------------------
# global mappigns defind in the script
#---------------------------------------------
my %lineMAPinfo;   # map line number with its info (weight grep rev1 rev2..)
my %revMAPcolor;   # maps revision with color coding
my %revMAPmatch;   # only stores value for matched revisions
my %revMAPcomment; # maps revision number to cvs comment for that version

# control character separator
my $ctrlA = chr(01);
my $grepquery;
my $revs;
my $id;
my $stemquery;
my $path;
my @colors;
my @revs;
my $yesgrep;

sub print_html();

#-------------
# start html
#-------------

print "Content-Type: text/html\n\n";
print "<html>\n";
print "<head>\n";

# ----------------------------------------
# print javascript for popups
# ----------------------------------------
open JS, "popup.js";
print <JS>;
close JS;

# ----------------------------------------
# parse parameters
# ----------------------------------------
if (param()) {
    parse_input();
    query();
    print_html();
}

sub print_javascript {
    my ($root, $pkg, $fileid) = @_;
    # ----------------------------------------
    # print javascript for calling popups in
    # shorthand notation
    # ----------------------------------------
    print <<_SCRIPT_;
<script language="JavaScript">

function l(targetObjectId, event) {
  locking(targetObjectId, event);
  return false;
}
  
function s(targetObjectId, event) {
  return !showPopup(targetObjectId, event);
}
  
function h() {
  hideCurrentPopup();
  return false;
}

function c(targetObjectId, event, line, rev){
    var link = "./Compare.cgi?root=$root&pkg=$pkg&fileid=$fileid&short=1&version="+ rev;
    if (showPopup(targetObjectId, event)) {
       link = link+"#"+line;
    }

    if (parent.frames[2].location.href != link) {
       parent.frames[2].location.href=link;
    }
    return false;
}

function o(line) {
    var link = "$source$passparam"+"#"+line;
    if (parent.frames[2].location.href != link) {
        parent.s.location.href = link;
    }
    return false;
}   
</script>
_SCRIPT_
}


sub parse_input {
    if (param()) {
        my $dump = param("dump");
        $id = param("id");
        my $did = Cvssearch::encode($id);

        $passparam = "?id=$did&dump=$dump";
        $id = Cvssearch::decode($id);
        my $found = Cvssearch::findfile($dump,$id);
        if (!$found){
            error("Page expired");
        }
        
        my @entries = split /\n/, $found;
        my $query = shift @entries;    #query
        $stemquery = shift @entries; #stemquery
        
        $grepquery = shift @entries;
        shift @entries;             #id
        $path = shift @entries;
        $revs = shift @entries;     #revs
        
        #go through each line entry
        foreach (@entries) {
            my @info = split /\s/, $_;
            my $line = shift @info;
            my $together = join ' ', @info;
            $lineMAPinfo{$line} = $together;
        }
    } else {
        error("Incorrect Parameters");
    }
}


sub query {
    #-------------------------------
    # query cvsquery for comments
    #-------------------------------
   
    my ($root, $db, $fileid) = split /\s/, $id;
    $db = Cvssearch::decode($db);
    $db =~tr/\_/\//;
    print_javascript($root, $db, $fileid);
    my $querystr = "$cvsquery $root $db";

    @revs = split /\s/, $revs;
    my $first = shift @revs;
    if($first ne "grep"){
        push @revs, $first;
    } else {
        $yesgrep = "yes";
    }
    foreach(@revs){
        $querystr .= " -c $fileid $_";
    }
    
    my $result = `$querystr`;
    my @comments = split /$ctrlA/, $result;
    
    my $i=0;
    foreach (@revs){
        my $currev = $_;
        my $curcomment = $comments[$i];
        chomp $curcomment;
        $curcomment = Entities::encode_entities($curcomment);
        if($curcomment =~/$grepquery/i){
			$revMAPmatch{$currev} = $currev;
		}
		$revMAPcomment{$currev} = $curcomment;
		#$curcomment = Cvssearch::highlightquery($curcomment, $grepquery);
		$i++;
	}
    @revs = keys %revMAPmatch;
    @revs = sort {Cvssearch::cmp_cvs_version($a, $b)} @revs;

    @colors = get_colors(@revs);
}


sub get_colors {
    my @revs = @_;
    if (@revs){
    	return Cvssearch::getSpectrum($#revs+1);
    }
}

sub print_html() {
    # ----------------------------------------
    # the common style sheet
    # ----------------------------------------
    Cvssearch::print_style_sheet();

    # ----------------------------------------
    # more style sheet that is page specific
    # ----------------------------------------
    print "<style type=\"text/css\">\n";
    print "A:link, A:active, A:visited { text-decoration:none;color:black;}\n";
	foreach (my $i=0;$i<$#revs+1;$i++){
		$revMAPcolor{$revs[$i]} = $colors[$i];
		my $ch = toChar($revs[$i]); # need to convert digits to alphabets since netscape doesn't understand digit id
        $ch =~ tr/\./-/;
		print ".$ch {background-color:$colors[$i];}\n";
	}
    print "</style>\n";
    
    # ----------------------------------------
    # body begins here
    # ----------------------------------------
    print "</head>\n";
    print "<body>\n";
	
	#---------------------------
	#write comments into divs
	#---------------------------
	
	print "<!-- keep the popup divs as the first things on the page or else ";
    print "MSIE 5 on the mac sometimes has trouble rendering them on top of text -->\n";
	print "<DIV onclick='event.cancelBubble = true;' class=popup id='grep'>";
    print "grep matched <b>$grepquery</b> on this line.</DIV>\n";
    
	my $i=0;
	foreach (@revs){
		my $currev = $_;
		my $curcomment = $revMAPcomment{$currev};
        $curcomment = Cvssearch::highlightquery($curcomment, $grepquery);
		my $ch = toChar($currev); # need to convert digits to alphabets since netscape doesn't understand digit id
		print "<DIV onclick='event.cancelBubble = true;' class=popup id='$ch'><pre><b>Rev:$currev</b>\n$curcomment</pre></DIV>\n";
		$i++;
	}

	# ----------------------------------------
	# display file heading
	# ----------------------------------------
	filename($stemquery);
	
    # ----------------------------------------
    # print a row of tags at the top
    # ----------------------------------------
    print "<pre>";
    
	foreach ($i=0;$i<$#revs+1;$i++){
		$revMAPcolor{$revs[$i]} = $colors[$i];
		my $ch = &toChar($revs[$i]); # need to convert digits to alphabets since netscape doesn't understand digit id
        my $ch1 = $ch;
        $ch1 =~ tr/\./-/;
		print "<a href=# ";
        print "onclick=\"return l('$ch',event);\" onmouseover=s('$ch',event); onmouseout=h();>";
		if($revMAPmatch{$revs[$i]}){
			print "<span class=$ch1>$revs[$i]</span>";
		}
		print "</a>&nbsp;";
	}
	print "</pre>\n";
	
    # ----------------------------------------
    # print line#, C/G's, source code link
    # ----------------------------------------
	print "<table cellSpacing=0 cellPadding=0 width=100% border=0>\n";
	my @file = `cat $path`;
	$i=1;                       #line index
	if($yesgrep){
		push @revs, "grep";
	}
    
    my $show_gap = 0;
    
	foreach(@file){
		s/\n//g;
		my $line = $_;
		if($lineMAPinfo{$i}){
            $show_gap = 1;
			$line = Entities::encode_entities($line);
			print "<tr>";
			print "<td><pre><a name=$i>$i</a></td>";
            
			my $info = $lineMAPinfo{$i};
			my @info = split / /, $info;
			my $weight = shift @info;

			print "<td><pre>";
			my $flag = 1;
            
			for(my $j=0;$j<=$#revs;$j++){
				my $toprev = $revs[$j];
				my $found = 0;
				$flag *= -1;
				foreach (@info){
					if($toprev eq $_){
						my $currev = $_;
						my $color = $revMAPcolor{$currev};
                        my $ch = toChar($currev); # need to convert digits to alphabets since netscape doesn't understand digit id
                        print "<a href=#$i ";
						if($revMAPmatch{$currev}){
                            print "onclick=\"return c('$ch', event, $i, \'$currev\');\" onmouseover=s('$ch',event); onmouseout=h();>";
                            if ($color) {
                                # need to convert digits to alphabets since netscape doesn't understand digit id
                                my $ch1 = &toChar($currev); 
                                $ch1 =~ tr/\./-/;
                                print "<span class=$ch1>";
                            } 
                            print "C";
                            if ($color) {
                                print "</span>";
                            }
						}elsif($currev eq "grep"){
                            print "onclick=\"return l('$ch', event);\" onmouseover=s('$ch',event); onmouseout=h();>";
							print "G";
						}
						print "</a>";
						$found = 1;
					}
				}
				if($found==0){
					if($flag==1){
						print " ";
					}else{
						print "<span class=g> </span>";
					}
				}
			}
            
			print "</td>";
			
			my $color = Cvssearch::get_color($weight, 150);
			$line = Cvssearch::highlightquery($line, $grepquery);
            my $space = "";
            if (length($line) == 0) {
                $space = " ";
            }
			print "<td bgcolor=$color><pre><a href=# onclick=\"return o($i);\">$line$space</a></td></tr>\n";
		}
		if ($lineMAPinfo{$i+1} > $lineMAPinfo{$i} && $show_gap != 0) {
            print "<tr><td></td><td><pre>";
			my $flag = 1;
			for(my $j=0;$j<=$#revs;$j++){
                $flag *= -1;
                if($flag==1){
                    print " ";
                }else{
                    print "<span class=g> </span>";
                }
            }
            print "</td><td></td></tr>\n";
            #            print "<tr><td colspan=3><div style=\"height:5px;\"> </div></td></tr>\n";
		}
		$i++;
	}
	print "</table>";
    print "</body></html>";
}



#--------------------------
# sub functions
#--------------------------

sub filename{
	my ($name) = @_;
	my $num = scalar(keys %lineMAPinfo);
	print Cvssearch::fileheader("<b>$num</b> Matched lines for <b>$name</b>",
                                "Move over C/G to see comment; Click C to see original commit below; Click matched line to see its context below");
}

#--------------------------------
# replace digits with characters
# 0->A 1->B 2->C ..etc
#--------------------------------
sub toChar{
	my ($word) = @_;
	$word =~ s/(\d)/@{[chr($1+65)]}/ig;
	return $word;
}

#------------------
# display errors
#------------------
sub error{
	my ($mesg) = @_;
	print "<p><b class=red>$mesg</b>";
	exit(0);
}

