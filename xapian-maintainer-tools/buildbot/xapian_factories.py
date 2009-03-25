# -*- python -*-
# ex: set syntax=python:

from buildbot.process import step, factory

from tarsource import Tar

class Bootstrap(step.ShellCommand):
    name = "bootstrap"
    haltOnFailure = 1
    description = ["bootstrapping"]
    descriptionDone = ["bootstrap"]
    command = ["./bootstrap"]

class CleanInstall(step.ShellCommand):
    name = "cleaninstall"
    haltOnFailure = 1
    description = ["cleaninstall"]
    descriptionDone = ["cleaninstall"]
    command = ['rm', '-rf', 'install']

class Install(step.ShellCommand):
    name = "install"
    haltOnFailure = 1
    description = ["install"]
    descriptionDone = ["install"]
    command = ['make', 'install']

class MakeWritable(step.ShellCommand):
    """Step which ensures that the permissions are writable on all
    subdirectories.
    """
    name = "make writable"
    haltOnFailure = 1
    description = ["making writable"]
    descriptionDone = ["made writable"]
    command = ["chmod", "-R", "+w", "."]

def gen_svn_updated_factory(baseURL):
    """
    Make a factory for doing HEAD build from SVN, but without cleaning
    first.  This build is intended to catch commonly made mistakes quickly.
    """
    f = factory.BuildFactory()
    f.addStep(step.SVN, baseURL=baseURL, defaultBranch='trunk', mode="update")
    f.addStep(Bootstrap)
    f.addStep(step.Configure)
    f.addStep(step.Compile)
    f.addStep(step.Test, name="check", command=("make", "check", "XAPIAN_TESTSUITE_OUTPUT=plain", "VALGRIND="))
    return f

def gen_svn_updated_factory2(baseURL):
    """
    Make a factory for doing HEAD build from SVN, but without cleaning
    first.  This build is intended to catch commonly made mistakes quickly.
    This factory also runs audit.py and publishes the result.
    """
    f = factory.BuildFactory()
    f.addStep(step.SVN, baseURL=baseURL, defaultBranch='trunk', mode="update")
    f.addStep(step.ShellCommand, command = ["python", 'audit.py'], workdir='build/xapian-maintainer-tools')
    f.addStep(step.ShellCommand, command = ["mv", 'copyright.csv', 'fixmes.csv', '/home/xapian-buildbot/pub/http/'], workdir='build/xapian-maintainer-tools')
    f.addStep(step.ShellCommand, command = ["chmod", '644', '/home/xapian-buildbot/pub/http/fixmes.csv', '/home/xapian-buildbot/pub/http/copyright.csv'])
    f.addStep(Bootstrap)
    f.addStep(step.Configure)
    f.addStep(step.Compile)
    f.addStep(step.Test, name="check", command=("make", "check", "XAPIAN_TESTSUITE_OUTPUT=plain", "VALGRIND="))
    return f

def gen_svn_updated_factory3(baseURL):
    """
    Make a factory for doing HEAD build from SVN, but without cleaning
    first.  This build is intended to catch commonly made mistakes quickly.
    This build runs with --disable-documentation, so the documentation building
    tools aren't required.
    """
    f = factory.BuildFactory()
    f.addStep(step.SVN, baseURL=baseURL, defaultBranch='trunk', mode="update")
    f.addStep(Bootstrap)
    f.addStep(step.Configure, command=("sh", "configure",
                                       "--disable-documentation",))
    f.addStep(step.Compile)
    f.addStep(step.Test, name="check", command=("make", "check", "XAPIAN_TESTSUITE_OUTPUT=plain", "VALGRIND="))
    return f


def gen_svn_gccsnapshot_updated_factory(baseURL):
    """
    Make a factory for doing HEAD build from SVN, but without cleaning
    first, using gcc snapshot.  Also uses compiles with logging and assertions.
    """
    f = factory.BuildFactory()
    f.addStep(step.SVN, baseURL=baseURL, defaultBranch='trunk', mode="update")
    f.addStep(Bootstrap)
    f.addStep(step.Configure,
              command=("sh", "configure", "--enable-assertions",
                       "--enable-log", "CXX=/usr/lib/gcc-snapshot/bin/g++",
                       "CC=/usr/lib/gcc-snapshot/bin/gcc",
                      ))
    f.addStep(step.Compile)
    f.addStep(step.Test, name="check", command=("make", "check", "XAPIAN_TESTSUITE_OUTPUT=plain", "VALGRIND="))
    return f

def gen_svn_debug_updated_factory(baseURL, *opts):
    """
    Make a factory for doing a debug HEAD build from SVN, but without cleaning
    first.  This build is intended to catch commonly made mistakes quickly.
    """
    f = factory.BuildFactory()
    f.addStep(step.SVN, baseURL=baseURL, defaultBranch='trunk', mode="update")
    f.addStep(Bootstrap)
    f.addStep(step.Configure, command=("sh", "configure", ) + opts)
    f.addStep(step.Compile)
    f.addStep(step.Test, name="check", command=("make", "check", "XAPIAN_TESTSUITE_OUTPUT=plain", "VALGRIND="))
    return f

def gen_tarball_updated_factory(rooturl):
    """
    Make a factory for doing builds from tarballs.
    """
    f = factory.BuildFactory()
    f.addStep(step.ShellCommand, command = ["python", "-c", "import urllib2;open('get_tarballs.py', 'wb').write(urllib2.urlopen('%s').read())" %
              'http://svn.xapian.org/*checkout*/trunk/xapian-maintainer-tools/buildbot/scripts/get_tarballs.py'], workdir='.', haltOnFailure=True)
    f.addStep(step.ShellCommand, command = ["python", 'get_tarballs.py'], workdir='.', haltOnFailure=True)
    f.addStep(step.Configure, workdir='build/xapian-core')
    f.addStep(step.Compile, workdir='build/xapian-core')
    f.addStep(step.Test, workdir='build/xapian-core', name="check", command=("make", "check", "XAPIAN_TESTSUITE_OUTPUT=plain", "VALGRIND="))
    f.addStep(step.Configure, workdir='build/xapian-omega', command=("python", "runconfigure.py"))
    f.addStep(step.Compile, workdir='build/xapian-omega')
    f.addStep(step.Test, workdir='build/xapian-omega', name="check", command=("make", "check", "XAPIAN_TESTSUITE_OUTPUT=plain", "VALGRIND="))
    f.addStep(step.Configure, workdir='build/xapian-bindings', command=("python", "runconfigure.py"))
    f.addStep(step.Compile, workdir='build/xapian-bindings')
    f.addStep(step.Test, workdir='build/xapian-bindings', name="check", command=("make", "check", "XAPIAN_TESTSUITE_OUTPUT=plain", "VALGRIND="))
    return f

def gen_svn_updated_valgrind_factory(baseURL):
    """
    Factory for doing HEAD build from SVN, without cleaning first, and using
    valgrind to check.  This one is much more expensive, so should be run with
    a higher stable time.
    """
    f = factory.BuildFactory()
    f.addStep(step.SVN, baseURL=baseURL, defaultBranch='trunk', mode="update")
    f.addStep(Bootstrap)
    f.addStep(step.Configure)
    f.addStep(step.Compile)

    for target in ("check-none", "check-inmemory", "check-remoteprog",
                   "check-flint"):
        f.addStep(step.Test, name=target, command=("make", target, "XAPIAN_TESTSUITE_OUTPUT=plain", "VALGRIND=/home/olly/install/bin/valgrind"), workdir='build/xapian-core')

    # Currently, valgrind incorrectly reports leaked memory for the remotetcp
    # backend, so check that one without using valgrind.
    f.addStep(step.Test, name="check-remotetcp", command=("make", "check-remotetcp", "XAPIAN_TESTSUITE_OUTPUT=plain", "VALGRIND=/home/olly/install/bin/valgrind"), workdir='build/xapian-core')

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
    f.addStep(step.SVN, baseURL=baseURL, defaultBranch='trunk', mode="clobber")
    f.addStep(Bootstrap)
    f.addStep(step.Configure, command = ["xapian-maintainer-tools/buildbot/scripts/configure_with_prefix.sh"])
    extraargs = (
        "XAPIAN_TESTSUITE_OUTPUT=plain", "VALGRIND="
    )
    f.addStep(step.Compile, command=("make",) + extraargs)
    # Don't bother running check as a separate step - all the checks will be
    # done by distcheck, anyway.  (Running it as a separate step _does_ check
    # that the tests work in a non-VPATH build, but this is tested by other
    # factories, anyway.)
    #f.addStep(step.Test, name="check", command=("make", "check") + extraargs)
    f.addStep(step.Test, name="distcheck", command=("make", "distcheck") + extraargs, workdir='build/xapian-core')
    f.addStep(step.Test, name="distcheck", command=("make", "distcheck") + extraargs, workdir='build/xapian-applications/omega')

    # Have to install the core for distcheck to pass on the bindings.
    f.addStep(step.Test, name="install", command=("make", "install") + extraargs, workdir='build/xapian-core')
    f.addStep(step.Test, name="distcheck", command=("make", "distcheck") + extraargs, workdir='build/xapian-bindings')
    return f

def gen_svn_updated_win_factory(baseURL):
    """
    Factory for doing a windows build from an SVN checkout, without cleaning first.
    """
    f = factory.BuildFactory()
    f.addStep(step.SVN, baseURL=baseURL, defaultBranch='trunk', mode="update")
    f.addStep(step.ShellCommand, command="xapian-maintainer-tools\\buildbot\\scripts\\prepare_build.bat")

    # Compile core: we use a .bat file to get vsvars32.bat to run before the
    # command.
    env = {}
    f.addStep(step.Compile, workdir="build/xapian-core/win32",
              command="..\\..\\xapian-maintainer-tools\\buildbot\\scripts\\compile_with_vc7.bat",
              env=env)

    return f

def gen_tarball_updated_win_factory(rooturl):
    """Make a factory for doing builds from tarballs on windows.

    """
    f = factory.BuildFactory()
    f.addStep(step.ShellCommand, command = ["python", "-c", "import urllib2;open('get_tarballs.py', 'wb').write(urllib2.urlopen('%s').read())" %
              'http://svn.xapian.org/*checkout*/trunk/xapian-maintainer-tools/buildbot/scripts/get_tarballs.py'], workdir='.', haltOnFailure=True)
    f.addStep(step.ShellCommand, command = ["python", 'get_tarballs.py'], workdir='.', haltOnFailure=True)
    f.addStep(step.Compile, workdir='build/xapian-core/win32', command=("compile_with_vc7.bat"))
    return f

all = []
for gen in dir():
    if gen.startswith('gen_'):
        all.append(gen[4:])
        locals()[gen[4:]] = locals()[gen]
