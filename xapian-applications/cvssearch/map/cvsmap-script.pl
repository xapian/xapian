# -* perl *-

open(PWD, "pwd|");
chomp($save_dir = <PWD>);
close PWD;
$curr_dir=$save_dir;

@file_types=qw(cc h cpp c C);

$list_file="/tmp/list";
$time_file="time";
$delta_time = 0;
unlink $time_file;
unlink $list_file;

if ($#ARGV < 0) {
    print "Usage $0 applications.txt\n";
    exit 1;
} else {
    open(APPS, "<$ARGV[0]");
    open(TIME, ">$time_file");

    while (<APPS>) {
        $line=chomp;
        ($app_name, @reverse_path) = reverse(split(/[\/\ ]+/));

        $path = $app_name;
        $name = $app_name;
        if ($#reverse_path >= 0 ) {
            for ($i = 0; $i <= $#reverse_path; ++$i) {
                $path = $reverse_path[$i] . '/' . $path;
                $name = $reverse_path[$i] . '_' . $name;
            }
        }
        if ($name ne "" ) {
            system ("cvs checkout $path 2>/dev/null");
            print "$path\n";
            if (chdir $path) {
                unlink $list;
		$found_files = 0;
                open(LIST, ">$list_file");
                for ($i = 0; $i <= $#file_types; ++$i) {
                    open(FIND_RESULT, "find . -name \"*.$file_types[$i]\"|");
                    while (<FIND_RESULT>) {
			$found_files = 1;
                        print LIST $_;
                    }
                    close(FIND_RESULT);
                }
                close(LIST);

		if ($found_files) {
	                print TIME "$path", "\n";
        	        print TIME "Started  @ ", `date`;
               		$start_date = time;
	                system ("cat $list_file\|xargs cvsmap \$\@ -f1 $save_dir/$name.db -f2 $save_dir/$name.offset");
        	        print TIME "Finished @ ", `date`;
	                $delta_time += time - $start_date;
	                print TIME "\n";
		}
               chdir $curr_dir || die "can't change back to $curr_dir\n";
            }
        }
    }
    print TIME "Operation Time: $delta_time Seconds \n";
    close(APPS);
    close(REC);
}


