# -*- python -*-
# ex: set syntax=python:

from buildbot.process import step, factory

class Bootstrap(step.ShellCommand):
    name = "bootstrap"
    haltOnFailure = 1
    description = ["bootstrapping"]
    descriptionDone = ["bootstrap"]
    command = ["./bootstrap"]

class MakeWritable(step.ShellCommand):
    """Step which cleans all subdirectories, ensuring that the permissions are
    suitable first.
    """
    name = "make writable"
    haltOnFailure = 1
    description = ["making writable"]
    descriptionDone = ["made writable"]
    command = ["chmod", "-R", "+w", "."]

class GetTarball(step.ShellCommand):
    pass # FIXME - implement this


def gen_svn_updated_factory(baseURL):
    """
    Make a factory for doing HEAD build from SVN, but without cleaning
    first.  This build is intended to catch commonly made mistakes quickly.
    """
    f = factory.BuildFactory()
    f.addStep(step.SVN, baseURL=baseURL, mode="update")
    f.addStep(Bootstrap)
    f.addStep(step.Configure)
    f.addStep(step.Compile)
    f.addStep(step.Test, name="check", command=("make", "check", "XAPIAN_TESTSUITE_OUTPUT=plain", "VALGRIND="))
    return f

def gen_svn_updated_valgrind_factory(baseURL):
    """
    Factory for doing HEAD build from SVN, without cleaning first, and using
    valgrind to check.  This one is much more expensive, so should be run with
    a higher stable time.
    """
    f = factory.BuildFactory()
    f.addStep(step.SVN, baseURL=baseURL, mode="update")
    f.addStep(Bootstrap)
    f.addStep(step.Configure)
    f.addStep(step.Compile)

    for target in ("check-void", "check-inmemory", "check-remote",
                   "check-remoteprog", "check-flint", "check-quartz"):
        f.addStep(step.Test, name=target, command=("make", target, "XAPIAN_TESTSUITE_OUTPUT=plain"))

    # Currently, valgrind incorrectly reports leaked memory for the remotetcp
    # backend, so check that one without using valgrind.
    f.addStep(step.Test, name="check-remotetcp", command=("make", "check-remotetcp", "XAPIAN_TESTSUITE_OUTPUT=plain", "VALGRIND="))

    return f

def gen_svn_clean_factory(baseURL):
    """
    Factory for doing HEAD build from a clean SVN checkout.  This build also
    performs a "make distcheck", so should catch problems with files which have
    been missed from the distribution.  This one is much more expensive, so
    should be run with a higher stable time.
    """
    f = factory.BuildFactory()
    f.addStep(MakeWritable, workdir='.')
    f.addStep(step.SVN, baseURL=baseURL, mode="clobber")
    f.addStep(Bootstrap)
    f.addStep(step.Configure)
    f.addStep(step.Compile)
    f.addStep(step.Test, name="check", command=("make", "check", "XAPIAN_TESTSUITE_OUTPUT=plain", "VALGRIND="))
    f.addStep(step.Test, name="distcheck", command=("make", "distcheck", "XAPIAN_TESTSUITE_OUTPUT=plain", "VALGRIND="))
    return f

def gen_svn_updated_win_factory(baseURL):
    """
    Factory for doing a windows build from an SVN checkout, without cleaning first.
    """
    f = factory.BuildFactory()
    f.addStep(step.SVN, baseURL=baseURL, mode="update")
    return f

def gen_tarball_factory(tarball_root):
    # Factory for doing build from tarballs.
    tarballs = [
        'xapian-core',
        'xapian-bindings',
        'xapian-omega',
    ]
    f = factory.BuildFactory()
    #f.addStep(GetTarball, root=tarball_root, tarballs=tarballs)
    return f

all = []
for gen in dir():
    if gen.startswith('gen_'):
        all.append(gen[4:])
        locals()[gen[4:]] = locals()[gen]
