## Process this file with automake to produce Makefile.in

# NB you also need to update this list in buildall
m4_files = cme_find_hstrerror.m4 \
	   cme_find_socketpair.m4 \
	   rjb_find_gethostent.m4 \
	   rjb_find_stlport.m4 \
	   rjb_check_berkeleydb.m4

EXTRA_DIST = $(m4_files) acinclude.m4 genacinclude.sh

# make acinclude.m4 depend on Makefile.am so that removing a file from the
# list in Makefile.am causes acinclude.m4 to be regenerated
acinclude.m4: $(m4_files) Makefile.am genacinclude.sh
	./genacinclude.sh @m4_files@
