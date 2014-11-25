# -*- python -*-
# ex: set syntax=python:

from buildbot.process import factory
from buildbot.steps import source, shell, slave
from datetime import date

xapian_config_arg = 'XAPIAN_CONFIG=../xapian-core/xapian-config'

def Bootstrap():
    return shell.ShellCommand(
        name = "bootstrap",
        haltOnFailure = 1,
        description = ["bootstrapping"],
        descriptionDone = ["bootstrap"],
        command = ["./bootstrap"],
    )

def CleanInstall():
    return shell.ShellCommand(
        name = "cleaninstall",
        haltOnFailure = 1,
        description = ["cleaninstall"],
        descriptionDone = ["cleaninstall"],
        command = ['rm', '-rf', 'install'],
    )

def Install():
    return shell.ShellCommand(
        name = "install",
        haltOnFailure = 1,
        description = ["install"],
        descriptionDone = ["install"],
        command = ['make', 'install'],
    )

def MakeWritable():
    """
    Step which ensures that the permissions are writable on all
    subdirectories.
    """
    return shell.ShellCommand(
        name = "make writable",
        haltOnFailure = 1,
        description = ["making writable"],
        descriptionDone = ["made writable"],
        command = ["chmod", "-R", "+w", "."],
    )

def core_factory(baseURL, usedocs=False, configure=None, audit=False,
                 clean=False, nocheck = False, configure_opts=None):
    f = factory.BuildFactory()
    mode = "update"
    if clean:
        #f.addStep(MakeWritable, workdir='.')
        f.addStep(shell.ShellCommand(command = ["chmod", "-R", "+w", "."], workdir='.'))
        mode = "clobber"
    f.addStep(source.SVN(baseURL=baseURL, defaultBranch='trunk', mode=mode))
    if audit:
        f.addStep(shell.ShellCommand(command = ["python", 'audit.py'], workdir='build/xapian-maintainer-tools'))
        f.addStep(shell.ShellCommand(command = ["chmod", '644', 'copyright.csv', 'fixmes.csv'], workdir='build/xapian-maintainer-tools'))
        f.addStep(shell.ShellCommand(command = ["mv", 'copyright.csv', 'fixmes.csv', '/var/www/'], workdir='build/xapian-maintainer-tools'))

    f.addStep(Bootstrap())
    if configure:
        f.addStep(shell.Configure(command=configure))
    else:
        if configure_opts is None:
            configure_opts = []
        if not usedocs:
            configure_opts.append("--disable-documentation")
        if configure_opts:
            f.addStep(shell.Configure(command=["sh", "configure"] + configure_opts))
        else:
            f.addStep(shell.Configure())

    f.addStep(shell.Compile())
    if not nocheck:
        f.addStep(shell.Test(name="check", command=["make", "check", "XAPIAN_TESTSUITE_OUTPUT=plain", "VALGRIND="]))
    return f

def gen_svn_updated_factory(baseURL, usedocs=False, clean=False):
    """
    Make a factory for doing HEAD build from SVN, but without cleaning
    first.  This build is intended to catch commonly made mistakes quickly.
    """
    return core_factory(baseURL=baseURL, usedocs=usedocs, clean=clean)

def gen_svn_updated_factory_llvm(baseURL):
    """
    Make a factory for doing HEAD build from SVN, but without cleaning
    first.  This build is intended to catch commonly made mistakes quickly.
    """
    return core_factory(baseURL=baseURL, configure_opts=["CXX=/Developer/usr/llvm-gcc-4.2/bin/llvm-g++-4.2", "CC=/Developer/usr/llvm-gcc-4.2/bin/llvm-gcc-4.2"])

def gen_svn_updated_factory2(baseURL, configure_opts=[]):
    """
    Make a factory for doing HEAD build from SVN, but without cleaning
    first.  This build is intended to catch commonly made mistakes quickly.
    This factory also runs audit.py and publishes the result.
    """
    return core_factory(baseURL=baseURL, usedocs=False, audit=True,
                        configure_opts=configure_opts)

def gen_svn_updated_factory3(baseURL):
    """
    Make a factory for doing HEAD build from SVN, but without cleaning
    first.  This build is intended to catch commonly made mistakes quickly.
    This build runs with --disable-documentation, so the documentation building
    tools aren't required.
    """
    return core_factory(baseURL=baseURL, usedocs=False)

def gen_svn_gccsnapshot_updated_factory(baseURL):
    """
    Make a factory for doing HEAD build from SVN, but without cleaning
    first, using gcc snapshot.  Also uses compiles with logging and assertions.
    """
    return core_factory(baseURL=baseURL,
                        configure_opts=["--enable-assertions", "--enable-log", "CXX=/usr/lib/gcc-snapshot/bin/g++", "CC=/usr/lib/gcc-snapshot/bin/gcc",
        ])

def gen_svn_debug_updated_factory(baseURL, opts, nocheck=False):
    """
    Make a factory for doing a debug HEAD build from SVN, but without cleaning
    first.  This build is intended to catch commonly made mistakes quickly.
    """
    f = factory.BuildFactory()
    f.addStep(source.SVN(baseURL=baseURL,
                         defaultBranch='trunk',
                         mode="update"))
    f.addStep(Bootstrap())
    opts.append("--disable-documentation")
    f.addStep(shell.Configure(command = ["sh", "configure", ] + opts))
    f.addStep(shell.Compile())
    if not nocheck:
        f.addStep(shell.Test(name="check", command = ["make", "check", "XAPIAN_TESTSUITE_OUTPUT=plain", "VALGRIND="]))
    return f

def gen_tarball_updated_factory(rooturl, nocheck=False, omega=True, configure_opts=[]):
    """
    Make a factory for doing builds from tarballs.
    """
    configure_cmd = ["sh", "configure", ] + configure_opts
    f = factory.BuildFactory()
    f.addStep(shell.ShellCommand(command = ["python", "-c", "try: import urllib2 as u\nexcept: import urllib.request as u\nopen('get_tarballs.py', 'wb').write(u.urlopen('%s').read())" %
              'http://trac.xapian.org/export/HEAD/trunk/xapian-maintainer-tools/buildbot/scripts/get_tarballs.py'], workdir='.', haltOnFailure=True))
    f.addStep(shell.ShellCommand(command = ["python", 'get_tarballs.py', rooturl], workdir='.', haltOnFailure=True))
    f.addStep(shell.Configure(workdir='build/xapian-core', command=configure_cmd))
    f.addStep(shell.Compile(workdir='build/xapian-core'))
    if not nocheck:
        f.addStep(shell.Test(workdir='build/xapian-core', name="check", command = ["make", "check", "XAPIAN_TESTSUITE_OUTPUT=plain", "VALGRIND="]))
    if omega:
        f.addStep(shell.Configure(workdir='build/xapian-omega', command = ["./configure", xapian_config_arg] + configure_opts))
        f.addStep(shell.Compile(workdir='build/xapian-omega'))
        if not nocheck:
            f.addStep(shell.Test(workdir='build/xapian-omega', name="check", command = ["make", "check", "XAPIAN_TESTSUITE_OUTPUT=plain", "VALGRIND="]))
    f.addStep(shell.Configure(workdir='build/xapian-bindings', command = ["./configure", xapian_config_arg] + configure_opts))
    f.addStep(shell.Compile(workdir='build/xapian-bindings', command = ["make"]))
    if not nocheck:
        f.addStep(shell.Test(workdir='build/xapian-bindings', name="check", command = ["make", "check", "XAPIAN_TESTSUITE_OUTPUT=plain", "VALGRIND="]))
    # If everything passed, there's not much point keeping the build - we'd
    # delete the old build tree and download new tarballs next time anyway.
    f.addStep(slave.RemoveDirectory('build'))
    return f

def gen_svn_updated_valgrind_factory(baseURL, configure_opts=[]):
    """
    Factory for doing HEAD build from SVN, without cleaning first, and using
    valgrind to check.  This one is much more expensive, so should be run with
    a higher stable time.
    """
    f = factory.BuildFactory()
    f.addStep(source.SVN(baseURL=baseURL, defaultBranch='trunk', mode="update"))
    f.addStep(Bootstrap())
    configure_opts.append("--disable-documentation")
    f.addStep(shell.Configure(command = ["sh", "configure", "CXXFLAGS=-O0 -g"] + configure_opts))
    f.addStep(shell.Compile())

    f.addStep(shell.Test(name="check", command = ["make", "check", "XAPIAN_TESTSUITE_OUTPUT=plain"], workdir='build/xapian-core'))

    return f

def gen_svn_updated_lcov_factory(baseURL, configure_opts=[]):
    """
    Factory for doing HEAD build from SVN, without cleaning first, and using
    lcov to generate a coverage report.  This one is much more expensive, so
    should be run with a higher stable time.
    """
    f = factory.BuildFactory()
    f.addStep(source.SVN(baseURL=baseURL, defaultBranch='trunk', mode="update"))
    f.addStep(Bootstrap())
    f.addStep(shell.Configure(command = ["sh", "configure", "--enable-maintainer-mode", "--disable-shared", "--disable-documentation", "CXXFLAGS=-O0 --coverage", "VALGRIND=", "CCACHE_DISABLE=1"] + configure_opts, workdir="build/xapian-core"))
    f.addStep(shell.Compile(workdir="build/xapian-core"))
    f.addStep(shell.ShellCommand(command = ["make", "coverage-check", "GENHTML_ARGS=--html-gzip"], workdir="build/xapian-core", haltOnFailure=True))
    f.addStep(shell.ShellCommand(command = ["chmod", "-R", "a+rX", "lcov"], workdir="build/xapian-core", haltOnFailure=True))
    f.addStep(shell.ShellCommand(command = 'NOW=`date -u +%Y-%m-%d`; cp -a lcov/. /var/www/"$NOW" && ln -sfT "$NOW" /var/www/latest', workdir="build/xapian-core", haltOnFailure=True))

    return f

#### FIXME: factories beyond here not updated

def gen_svn_clean_dist_factory(baseURL):
    """
    Factory for doing HEAD build from a clean SVN checkout.  This build also
    performs a "make distcheck", so should catch problems with files which have
    been missed from the distribution.  This one is much more expensive, so
    should be run with a higher stable time.
    """
    f = factory.BuildFactory()
    f.addStep(MakeWritable, workdir='.')
    f.addStep(source.SVN, baseURL=baseURL, defaultBranch='trunk', mode="clobber")
    f.addStep(Bootstrap())
    f.addStep(step.Configure, command = ["xapian-maintainer-tools/buildbot/scripts/configure_with_prefix.sh"])
    extraargs = (
        "XAPIAN_TESTSUITE_OUTPUT=plain", "VALGRIND="
    )
    f.addStep(step.Compile, command = ["make",] + extraargs)
    # Don't bother running check as a separate step - all the checks will be
    # done by distcheck, anyway.  (Running it as a separate step _does_ check
    # that the tests work in a non-VPATH build, but this is tested by other
    # factories, anyway.)
    #f.addStep(step.Test, name="check", command = ["make", "check"] + extraargs)
    f.addStep(step.Test, name="distcheck", command = ["make", "distcheck"] + extraargs, workdir='build/xapian-core')
    f.addStep(step.Test, name="distcheck", command = ["make", "distcheck"] + extraargs, workdir='build/xapian-applications/omega')

    # Have to install the core for distcheck to pass on the bindings.
    f.addStep(step.Test, name="install", command = ["make", "install"] + extraargs, workdir='build/xapian-core')
    f.addStep(step.Test, name="distcheck", command = ["make", "distcheck"] + extraargs, workdir='build/xapian-bindings')
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
    f.addStep(shell.ShellCommand(command = ["python", "-c", "try: import urllib2 as u\nexcept: import urllib.request as u\nopen('get_tarballs.py', 'wb').write(u.urlopen('%s').read())" %
              'http://trac.xapian.org/export/HEAD/trunk/xapian-maintainer-tools/buildbot/scripts/get_tarballs.py'], workdir='.', haltOnFailure=True))
    f.addStep(shell.ShellCommand, command = ["python", 'get_tarballs.py', rooturl], workdir='.', haltOnFailure=True)
    f.addStep(shell.Compile, workdir='build/xapian-core/win32', command = ["compile_with_vc7.bat"])
    return f

all = []
for gen in dir():
    if gen.startswith('gen_'):
        all.append(gen[4:])
        locals()[gen[4:]] = locals()[gen]
