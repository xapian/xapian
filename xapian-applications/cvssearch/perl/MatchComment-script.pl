use strict;
use CGI ':all';
use Cvssearch;
use Entities;

sub print_javascript;
sub toChar;

my $ctrlA = chr(01);
my $ctrlC = chr(03);
my $cvsdata = Cvssearch::get_cvsdata();

# ----------------------------------------
# path variables
# ----------------------------------------
my $cvsquery = "./cvsquerydb";

# ----------------------------------------
# three frames
# ----------------------------------------
my $top = "TopComment.cgi";
my $match = "MatchComment.cgi";
my $source = "SourceComment.cgi";

# ----------------------------------------
# start html
# ----------------------------------------
print "Content-Type: text/html\n\n";
print "<html>\n";
print "<head>\n";
Cvssearch::print_style_sheet();
if (param()) {
    my $id = param("id");
    my $root = Cvssearch::sanitise_root(param("root"));
    my $pkg = param("pkg");
    my $symbol = param("symbol");
### Amir: HACK TO SHOW ALL COMMIT LINES
    $symbol = "";
    my $passparam = "?id=$id&root=$root&pkg=$pkg";

    my @file_ids;
    my @file_names;
    my @revisions;
    open (QUERY, "$cvsquery $root $pkg -A $id |");
    while (<QUERY>) {
        chomp;
        if (0) {
        } elsif (/$ctrlC/) {
            my @fields = split(/$ctrlC/);
            @file_ids   = (@file_ids,   $fields[0]);
            @file_names = (@file_names, $fields[1]);
            @revisions  = (@revisions,  $fields[2]);
        }
    }
    close (QUERY);

    if ($#file_ids < 0 || $#file_names < 0 || $#revisions < 0) {
        print "</head>\n";
        print "<body>\n";
        print "no nontrivial changes involved in this commit or this commit doesn't exist.\n";
        print "</body></html>";
        exit (0);
    }
    my @revisions_temp = @revisions;
    my @file_ids_temp = @file_ids;
    my @colors = Cvssearch::getSpectrum($#file_ids+1);

    my $query_string = "$cvsquery $root $pkg";
    while (@file_ids) {
        $query_string .= " -l ".(shift @file_ids)." ".(shift @revisions);
    }
    
    @file_ids = @file_ids_temp;
    @revisions = @revisions_temp;

    # ----------------------------------------
    # more style sheet that is page specific
    # ----------------------------------------
    print "<style type=\"text/css\">\n";
    print "A:link, A:active, A:visited { text-decoration:none;color:black;}\n";
	foreach (my $i=0;$i<=$#file_ids;$i++){
		my $ch = toChar($file_ids[$i]); # need to convert digits to alphabets since netscape doesn't understand digit id
		print ".$ch {background-color:$colors[$i];}\n";
	}
    print "</style>\n";

    # ----------------------------------------
    # this file_name contains the pkg name
    # ----------------------------------------

    print_javascript($root, $pkg, $symbol);
    print "</head>\n";
    print "<body>\n";

    print "Code shown below is what appears in the most recent version of the file(s).<br>";
    print "To see the original commit and how we propagated the code to the most recent version, click 'F'.<br>";
    print "Click on a line of code to see it in context below.<br>";
    print "To view another file in this package, click the package link at the top.<br>";
     

    print "<table cellspacing=\"0\" cellpadding=\"0\" width=\"100%\" border=\"0\">\n";

    my $revision  = shift @revisions;
    my $file_id   = shift @file_ids;
    my $file_name = shift @file_names; 
    if ($pkg == substr($file_name, 0, length($pkg))) {
        $file_name = substr($file_name, length($pkg)+1, length($file_name)-length($pkg)-1);
    }

    my $color     = shift @colors;
    my $class     = toChar($file_id);
    my $i = 1;
    my $has_something = 1;
    my $printed_something = 0;
    my $file_last_printed_line = 0;
    open (FILE, "<$cvsdata/$root/src/$pkg/$file_name");
    open (QUERY, "$query_string |");
    while (<QUERY>) {
        chomp;
        if (/$ctrlA/) {
            close (FILE);
            if ($#file_names >= 0) {
                $revision = shift @revisions;
                $file_id = shift @file_ids;
                $file_name = shift @file_names;
                if ($pkg == substr($file_name, 0, length($pkg))) {
                    $file_name = substr($file_name, length($pkg)+1, length($file_name)-length($pkg)-1);
                }

                $class     = toChar($file_id);
                $color = shift @colors;
                open (FILE, "<$cvsdata/$root/src/$pkg/$file_name");
                $i = 1;
                $has_something = 1;
                $file_last_printed_line = 0;
            }
        } else {
            my $line_index = $_;
            while ($i < $line_index) {
                my $temp = <FILE>;
                $i++;
            }
            my $line = <FILE>;
            chomp $line;
            my $space = "";
            if (length($line) == 0) {
                $space = " ";
            }

            $line = Entities::encode_entities($line);

            if ($symbol ne "") {
                my $old_line = $line;
                $line = Cvssearch::highlightquery($line, quotemeta $symbol);
                if ($line eq $old_line) {
                    $i++;
                    next;
                }
            }
            if ($has_something) {
                print "<tr><td colspan=3>&nbsp;</td></tr>\n";
                print "<tr><td colspan=3 class=\"s\">$file_name,&nbsp;revision:$revision</td></tr>\n";
                $has_something = 0;
            }
            $printed_something = 1;
            if ($file_last_printed_line + 1 < $i) {
                print "<tr><td colspan=3>&nbsp;</td></tr>\n";
            }
            print "<tr><td>$line_index </td><td>";
            print "<a href=# ";
            print "onclick=\"return c($line_index, $file_id, \'$revision\');\"><span class=\"$class\">F</span></a> ";
            print "</td><td class=\"t\"><pre><a href=# onclick=\"return o($file_id, \'$revision\', $line_index);\">$line$space</a></td></tr>\n";
            $file_last_printed_line = $i;
            $i++;
        }
    }
    close (QUERY);
    print "</table>\n";

    if ($printed_something == 0) {
        print "no symbols $symbol found\n";
    }
} else {
    print "</head>\n";
    print "<body>\n";
	print "need to pass cvs commit id, root, pkg as parameters";
}
print "</body></html>";


sub print_javascript {
    my ($root, $pkg, $symbol) = @_;
    # ----------------------------------------
    # print javascript for calling popups in
    # shorthand notation
    # ----------------------------------------
# ----------------------------------------
# print javascript for popups
# ----------------------------------------
open JS, "popup.js";
print <JS>;
close JS;

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

function c(line, fileid, rev){
    var link = "./Compare.cgi?root=$root&pkg=$pkg&fileid=" + fileid +"&short=1&version="+ rev + "#" + line;

    if (parent.frames[2].location.href != link) {
       parent.frames[2].location.href=link;
    }
    return false;
}

function o(fileid, revision, line) {
    var link = "$source?root=$root&pkg=$pkg" + "&revision=" + revision + "&fileid="+ fileid + "&symbol=$symbol#"+line;
    if (parent.frames[2].location.href != link) {
        parent.frames[2].location.href = link;
    }
    return false;
}   
</script>
_SCRIPT_
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
