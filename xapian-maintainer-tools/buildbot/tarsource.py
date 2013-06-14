# -*- python -*-
# ex: set syntax=python:

# Based on SVNPoller

from twisted.python import log
from twisted.internet import defer, reactor, utils
from twisted.internet.task import LoopingCall

from buildbot import util
from buildbot.changes import base
from buildbot.changes.changes import Change
from buildbot.process.buildstep import RemoteShellCommand
from buildbot.steps.source import Source

import re

tarlink_re = re.compile('<a href="([a-zA-Z0-9_\.-]+).tar.gz">')

def parsehtml(html, archives):
    max_revision = 0
    links = []
    for link in tarlink_re.findall(html):
        for archive in archives:
            if link.startswith(archive):
                revision = link[len(archive):]
                svnpos = revision.find('svn')
                if svnpos > 0:
                    revision = int(revision[svnpos + 3:])
                    if revision > max_revision:
                        links = []
                        max_revision = revision
                    if revision == max_revision:
                        links.append(link)
    if max_revision == 0:
        log.msg("No valid links found")
        return None
    elif len(links) == len(archives):
        log.msg("Parsing html index page: found all archives for revision: %d" % (max_revision, ))
        return (max_revision, links)
    else:
        log.msg("Latest revision (%d) is not complete: found the following links: %r" % (max_revision, links, ))
        return None

class TarPoller(base.ChangeSource, util.ComparableMixin):

    compare_attrs = ["tarball_root", "branch", "archives", "pollinterval",]
    last_revision = None
    loop = None
    parent = None
    working = False

    def __init__(self, tarball_root, branch=None, archives=None, pollinterval=60):
        self.tarball_root = tarball_root
        self.branch = branch
        self.archives = archives
        self.pollinterval = pollinterval
        self.loop = LoopingCall(self.checktar)

    def startService(self):
        log.msg("TarPoller(%s) starting" % self.tarball_root)
        base.ChangeSource.startService(self)
        # Don't start the loop just yet because the reactor isn't running.
        # Give it a chance to go and install our SIGCHLD handler before
        # spawning processes.
        reactor.callLater(0, self.loop.start, self.pollinterval)

    def stopService(self):
        log.msg("TarPoller(%s) shutting down" % self.tarball_root)
        self.loop.stop()
        return base.ChangeSource.stopService(self)

    def describe(self):
        return "TarPoller watching %s" % self.tarball_root

    def checktar(self):
        if self.working:
            log.msg("TarPoller(%s) overrun: timer fired but the previous "
                    "poll had not yet finished.")
            self.overrun_counter += 1
            return defer.succeed(None)
        self.working = True

        log.msg("TarPoller polling")
        d = utils.getProcessOutput("curl", ['-s', self.tarball_root], {})
        d.addCallback(parsehtml, self.archives)
        d.addCallback(self.create_changes)
        d.addCallback(self.submit_changes)

        d.addCallbacks(self.finished_ok, self.finished_failure)
        return d

    def create_changes(self, info):
        if info is None:
            # Got no consistent revision
            log.msg('TarPoller: got no consistent revision')
            return None
        (revision, links) = info
        if self.last_revision is None:
            log.msg('TarPoller: start revision is %r' % (revision,))
            self.last_revision = revision
            return None
        if self.last_revision == revision:
            # No change from last revision
            log.msg('TarPoller: still at previous revision: %r' % (revision,))
            return None
        self.last_revision = revision

        log.msg('TarPoller: new revision found: %r' % (revision,))
        c = Change(who='',
                   files=[],
                   comments='',
                   revision=revision,
                   branch=self.branch)
        c.links = links
        return c

    def submit_changes(self, change):
        if change is not None:
            self.parent.addChange(change)

    def finished_ok(self, res):
        log.msg("TarPoller finished polling")
        log.msg('_finished : %s' % res)
        assert self.working
        self.working = False
        return res

    def finished_failure(self, f):
        log.msg("TarPoller failed")
        log.msg('_finished : %s' % f)
        assert self.working
        self.working = False
        return None # eat the failure


class Tar(Source):
    name = "tar"
    def __init__(self, rooturl, archives, **kwargs):
        self.branch = None
        Source.__init__(self, **kwargs)
        self.rooturl = rooturl
        self.archives = archives
        self.working = False

    def computeSourceRevision(self, changes):
        if not changes: return None
        changelist = [c.revision for c in changes]
        if len(changelist) == 0: return None
        changelist.sort()
        return changelist[-1]

    def startVC(self, branch, revision, patch):
        d = utils.getProcessOutput("curl", ['-s', self.rooturl], {})
        d.addCallback(self.doStartVC, branch, revision, patch)
        d.addErrback(self.failed)

    def doStartVC(self, html, branch, revision, path):
        info = parsehtml(html, self.archives)

        if info is None:
            # Got no consistent revision
            log.msg('Tar: got no consistent revision')
            self.failed('Couldn\'t get consistent revision')
            return
        (revision, links) = info

        cmdargs = ['curl', '-s']
        for link in links:
            cmdargs.append('-O')
            cmdargs.append(self.rooturl + link + '.tar.gz')

        cmd = RemoteShellCommand('build', command=(cmdargs))
        self.startCommand(cmd)

