#!/bin/sh

run_prog() {
    preload="../testsuite/malloccheck.so"
    if test -r "$preload"
    then
        LD_PRELOAD="$preload"
	export LD_PRELOAD
	exec ./$1
    else
        echo "malloccheck.so not found" >&2
	exec ./$1
    fi
}

# this is a bit of a hack...
for x in * ; do
    case "$0" in 
        *run-$x) run_prog "$x" ;;
	*) ;;
    esac
done

echo "Failed to find test to run!" >&2
exit 1
