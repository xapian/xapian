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
</script>
_SCRIPT_



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
    @revs = sort {cmp_cvs_version($a, $b)} @revs;
    if (@revs){
    	@colors = Cvssearch::getSpectrum($#revs+1);
    }
    
    # ----------------------------------------
    # style sheet
    # ----------------------------------------
    print "<style type=text/css>\n";
    print "body {background-color:white;}\n";
    print ".g {background-color:#dddddd;}\n";
    print "A:link, A:active, A:visited { text-decoration:none;color:black;}\n";
    print ".popupLink { color: blue; outline: none;}\n";
    print ".popup { position:absolute; visibility:hidden; color:white;background-color:#3366cc;";
    print "layer-background-color:#3366cc;border:2px solid orange; padding: 3px; z-index: 10;}\n";
    print ".red {color:red;}\n";
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
		print "<span class=$ch1><a href=# ";
        print "onclick=\"return l('$ch',event);\" onmouseover=s('$ch',event); onmouseout=h();>";

		if($revMAPmatch{$revs[$i]}){
			print "<b>$revs[$i]</b>";
		}else{
			print "$revs[$i]";
		}
		print "</a></span> ";
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
			print "<td><pre>$i</td>";
            
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
                        $ch = &toChar($currev); # need to convert digits to alphabets since netscape doesn't understand digit id

                        if ($color) {
                            $ch1 = &toChar($currev); # need to convert digits to alphabets since netscape doesn't understand digit id
                            $ch1 =~ tr/\./-/;
                            print "<span class=$ch1>";
                        } 
                        print "<a href=# ";
                        print "onclick=\"return l('$ch',event);\" onmouseover=s('$ch',event); onmouseout=h();>";
						if($revMAPmatch{$currev}){
							print "C";
						}elsif($currev eq "grep"){
							print "G";
						}
						print "</a>";
                        if ($color) {
                            print "</span>";
                        }
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
			print "<td bgcolor=$color><pre><a href=\"$source$passparam#$i\" target=s>$line$space</a></td></tr>\n";
		}
		if ($lineMAPinfo{$i+1} > $lineMAPinfo{$i}) {
            print "<tr><td colspan=3><div style=\"height:5px;\"> </div></td></tr>\n";
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
	print Cvssearch::fileheader("<b>$num</b> Matched lines for <b>$name</b>", "Move over C/G to see comment; Click C/G to make it stick/unstick; Click matched line to see its context below");
    
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

### This function will be passed to the standard
### perl sort function for sort the cvs versions.
### ie. 1.10 is later than l.9
sub cmp_cvs_version
  {
      my $first = $_[0];
      my $second = $_[1];
      
      my @first = split(/\./, $first);
      my @second = split(/\./, $second);
      
      my $size = $#first;
      $size = $#second if ($#first > $#second); 
      
      for (my $i=0; $i<=$size; $i++) {
          if ($first[$i]>$second[$i]) {
              return 1;
          } elsif ($first[$i]<$second[$i]) {
              return -1;
          } else {
            next;
        }
    }
    
    if ($#first > $#second) {
        return 1;
    } elsif ($#first < $#second) {
        return -1;
    } else {
        return 0;
    }
}
