#-------------------------------------------------------------------
# some stuff about cvssearch
# This produces a html with matched lines only and it's revision 
# information.
#
# Author: Annie - anniec@cse.unsw.edu.au
# Date: Feb 16 2001
#-------------------------------------------------------------------


use CGI ':all';
use Cvssearch;
use Entities;

#-----------------
# paths
#-----------------
$source = "./Source.cgi";
$cvsquery = "./cvsquerydb";
$passparam = "";
#---------------------------------------------
# global mappigns defind in the script
#---------------------------------------------
#%lineMAPinfo - map line number with its info (weight grep rev1 rev2..)
#%revMAPcolor - maps revision with color coding
#%revMAPmatch - only stores value for matched revisions
#%revMAPcomment - maps revision number to cvs comment for that version

# control character separator
$ctrlA = chr(01);

#-------------
# start html
#-------------

print "Content-type:text/html\n\n";
print "<html>\n";
print "<head>\n";

# ----------------------------------------
# print javascript for popups
# ----------------------------------------
open (OUTPUT, "<./popup.js");
print <OUTPUT>;
close (OUTPUT);


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


#----------------------------------------
# parse parameters
#----------------------------------------
if(param()){
	$dump = param("dump");
	$id = param("id");
	$did = Cvssearch::encode($id);
	$passparam = "?id=$did&dump=$dump";
    
	$id = Cvssearch::decode($id);
	
	
	$found = Cvssearch::findfile($dump,$id);
	if (!$found){
        &error("Page expired");
	}
    
	@entries = split /\n/, $found;
	$query = shift @entries;    #query
	$stemquery = shift @entries; #stemquery
	$grepquery = shift @entries;
	shift @entries;             #id
	$path = shift @entries;
	$revs = shift @entries;     #revs
	
	#go through each line entry
	foreach (@entries){
		@info = split /\s/, $_;
		$line = shift @info;
		$together = join ' ', @info;
		$lineMAPinfo{$line} = $together;
	}
	#-------------------------------
	# query cvsquery for comments
	#-------------------------------
    
	($root, $db, $fileid) = split / /, $id;
    print_javascript($root, $db, $fileid);
	$querystr = "$cvsquery $root $db";
    
	@revs = split /\s/, $revs;
	$first = shift @revs;
	if($first ne "grep"){
		push @revs, $first;
	}else{
		$yesgrep = "yes";
	}
	foreach(@revs){
		$querystr .= " -c $fileid $_";
	}
    
	$result = `$querystr`;
	@comments = split /$ctrlA/, $result;
    
	$i=0;
	foreach (@revs){
		$currev = $_;
		$curcomment = $comments[$i];
		chomp $curcomment;
		$curcomment = Entities::encode_entities($curcomment);
		if($curcomment =~/$grepquery/i){
			$revMAPmatch{$currev} = $currev;
		}
		$revMAPcomment{$currev} = $curcomment;
		#$curcomment = highlightquery($curcomment);
		$i++;
	}
    @revs = keys %revMAPmatch;
    @revs = sort {Cvssearch::cmp_cvs_version($a, $b)} @revs;
    if (@revs){
    	@colors = Cvssearch::getSpectrum($#revs+1);
    }
    
    # ----------------------------------------
    # style sheet
    # ----------------------------------------
    Cvssearch::print_style_sheet();

    print "<style type=\"text/css\">\n";
    print "A:link, A:active, A:visited { text-decoration:none;color:black;}\n";
	foreach ($i=0;$i<$#revs+1;$i++){
		$revMAPcolor{$revs[$i]} = $colors[$i];
		$ch = toChar($revs[$i]); # need to convert digits to alphabets since netscape doesn't understand digit id
        $ch =~ tr/\./-/;
		print ".$ch {background-color:$colors[$i];}\n";
	}
    print "</style>\n";

    print "</head>\n";
    print "<body>\n";
	
	#---------------------------
	#write comments into divs
	#---------------------------
	
	print "<!-- keep the popup divs as the first things on the page or else ";
    print "MSIE 5 on the mac sometimes has trouble rendering them on top of text -->\n";
	print "<DIV onclick='event.cancelBubble = true;' class=popup id='grep'>";
    print "grep matched <b>$grepquery</b> on this line.</DIV>\n";
    
	$i=0;
	foreach (@revs){
		$currev = $_;
		$curcomment = $revMAPcomment{$currev};
        $curcomment = highlightquery($curcomment);
		$ch = &toChar($currev); # need to convert digits to alphabets since netscape doesn't understand digit id
		print "<DIV onclick='event.cancelBubble = true;' class=popup id='$ch'><pre><b>Rev:$currev</b>\n$curcomment</pre></DIV>\n";
		$i++;
	}
    
	#----------------
	#display file
	#----------------
	
	#filename
	&filename($stemquery);
	
    
    print "<pre>";
    
	foreach ($i=0;$i<$#revs+1;$i++){
		$revMAPcolor{$revs[$i]} = $colors[$i];
		$ch = &toChar($revs[$i]); # need to convert digits to alphabets since netscape doesn't understand digit id
        $ch1 = $ch;
        $ch1 =~ tr/\./-/;
		print "<a href=# ";
        print "onclick=\"return l('$ch',event);\" onmouseover=s('$ch',event); onmouseout=h();>";
		if($revMAPmatch{$revs[$i]}){
			print "<span class=$ch1>$revs[$i]</span>";
		}
		print "</a>&nbsp;";
	}
	print "</pre>\n";
	
	print "<table cellSpacing=0 cellPadding=0 width=100% border=0>\n";
	@file = `cat $path`;
	$i=1;                       #line index
	if($yesgrep){
		push @revs, "grep";
	}
	foreach(@file){
		s/\n//g;
		$line = $_;
		if($lineMAPinfo{$i}){
			$line = Entities::encode_entities($line);
			print "<tr>";
			print "<td><pre><a name=$i>$i</a></td>";
            
			$info = $lineMAPinfo{$i};
			@info = split / /, $info;
			$weight = shift @info;
			
			print "<td><pre>";
			$flag = 1;
            
			for($j=0;$j<=$#revs;$j++){
				$toprev = $revs[$j];
				$found = 0;
				$flag *= -1;
				foreach (@info){
					if($toprev eq $_){
						$currev = $_;
						$color = $revMAPcolor{$currev};
                        $ch = toChar($currev); # need to convert digits to alphabets since netscape doesn't understand digit id
                        print "<a href=#$i ";
						if($revMAPmatch{$currev}){
                            print "onclick=\"return c('$ch', event, $i, $currev);\" onmouseover=s('$ch',event); onmouseout=h();>";
                            if ($color) {
                                # need to convert digits to alphabets since netscape doesn't understand digit id
                                $ch1 = &toChar($currev); 
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
                    # this is removed for fixxing a bug.. don't uncomment this.
					# last if $toprev eq $currev;
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
			
			$color = Cvssearch::get_color($weight, 150);
			$line = &highlightquery($line);
            $space = "";
            if (length($line) == 0) {
                $space = " ";
            }
			print "<td bgcolor=$color><pre><a href=# onclick=\"return o($i);\">$line$space</a></td></tr>\n";
		}
		if ($lineMAPinfo{$i+1} > $lineMAPinfo{$i}) {
            print "<tr><td></td><td><pre>";
			$flag = 1;
			for($j=0;$j<=$#revs;$j++){
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
#	print "<pre>";
#	for($j=0;$j<60;$j++){
#		print "\n";	
#	}
#	print "</pre>";
}

print "</body></html>";


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

sub filename{
	my ($name) = @_;
	$num = scalar(keys %lineMAPinfo);
	print Cvssearch::fileheader("<b>$num</b> Matched lines for <b>$name</b>", "Move over C/G to see comment; Click C to see original commit below; Click matched line to see its context below");
    
}



#-----------------------------------
# highlightquery
# return line matched by query words
# make words matched by query bold
#-----------------------------------
sub highlightquery{
	my ($words) = @_;
	$words =~ s/($grepquery)/<b>\1<\/b>/ig;
	return $words;
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
	($mesg) = @_;
	print "<p><b class=red>$mesg</b>";
	exit(0);
}

