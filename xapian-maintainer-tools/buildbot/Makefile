# -*- makefile -*-

# This is a simple makefile which lives in a buildmaster/buildslave
# directory (next to the buildbot.tac file). It allows you to start/stop the
# master or slave by doing 'make start' or 'make stop'.

# The 'reconfig' target will tell a buildmaster to reload its config file.

all: stop start

start: stop
	cd /home/xapian-buildbot/master && \
	twistd --no_save -y buildbot.tac

stop:
	cd /home/xapian-buildbot/master && \
	if test -e twistd.pid; then kill `cat twistd.pid` || true; fi

reconfig:
	cd /home/xapian-buildbot/master && \
	if test -e twistd.pid; then \
	  kill -HUP `cat twistd.pid`; \
	fi

log:
	cd /home/xapian-buildbot/master && \
	tail -f twistd.log
