#! /bin/sh

run_prog() {
    preload="../testsuite/malloccheck.so"
    if test ! -f "$preload"
    then
        preload="../$preload"
    fi
    if test ! -f "$preload"
    then
        preload="../$preload"
    fi

    if test ! -f "$preload" || test "x$OM_NO_MALLOCCHECK" != "x"
    then
        preload=""
    fi

    # disable malloc checking for now until we fix it to work with
    # the new leak checking framework
    preload=""

    OM_DTD_PATH="${srcdir}/../indexer/indexgraph/omindexer.dtd"
    export OM_DTD_PATH
    if test -n "$preload"
    then
        LD_PRELOAD="$preload"
	export LD_PRELOAD
	exec $USE_GDB "$@"
    else
        #echo "malloccheck.so not found or disabled" >&2
	exec $USE_GDB "$@"
    fi
}

run_prog ./`echo $0|sed 's!.*run-!!'` "$@"

echo "Failed to find test to run!" >&2
exit 1
