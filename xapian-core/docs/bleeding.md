% Bleeding Edge


If you want a stable version of Xapian, we recommend using a
[released version](http://xapian.org/download.php).  But if you're happy to cope
with potential breakage and want to try the latest development code, or
do development yourself, you can access our version control system
which runs on Subversion (known as SVN for short.)



The Subversion repository includes
a complete history of the code, including that from the original
Open Muscat project (when converting to SVN we dropped old nightly snapshot
tags and a few
others which it seems highly unlikely anyone would find useful - these
can still be used in the now frozen Xapian CVS tree - see below.)
Additionally, we've recreated copy and rename operations into the Subversion
history (CVS doesn't support copy or rename directly.)

## Access Details

Note: If you just want to look at the history of a few files, you may find
it easier and quicker to 
[browse our SVN repository online](http://svn.xapian.org/).


To get the very latest version of Xapian from our repository, follow these
steps:

1. `svn co svn://svn.xapian.org/xapian/trunk xapian`
2. Read the "Building from SVN" section in [`xapian-core/HACKING`](http://svn.xapian.org/trunk/xapian-core/HACKING?view=co) - in particular make sure you have the required tools installed.
3. In the newly created `xapian` directory, run the command
    `./bootstrap` - this will run various developer tools to produce a
    source tree like you'd get from unpacking release source tarballs.
4. `bootstrap` will create a top level `configure` script,
    which you can use to configure the whole source tree together.
5. If you're looking to do development work on Xapian, then the rest of
    `xapian-core/HACKING` is recommended reading.

The latest Search::Xapian (Perl bindings for Xapian) development sources are
now in the tree checked out by the above command.

## Snapshots

We plan to set up an automatic snapshot system which will try to compile and
run the library testsuite every night, and upload a snapshot if all tests pass.
This is not currently operational, but you can
[download completely untested snapshots](http://www.oligarchy.co.uk/xapian/trunk/),
which are generated once an hour (so long as the code in SVN isn't too
broken for even `make dist` to work).


The snapshots are built automatically on various different platforms - you
can view the results of these builds in [our buildbot](http://buildbot.xapian.org/)
and also MinGW and MSVC build at
[Enfold Systems' buildbot](http://buildbot.enfoldsystems.com/xapian/).

## CVS

Prior to April 2005 we used CVS as our version control system.  The SVN tree
contains the full history, except some useless really old tags weren't
converted.  The (now frozen) Xapian CVS tree can still be browsed online
at <http://cvs.xapian.org/xapian/> should you really want to (we've not
made this a link, to try to avoid people browsing it without really reading
this paragraph and getting confused - such people almost certainly want to
[browse our SVN repository online](http://svn.xapian.org/) instead!)
