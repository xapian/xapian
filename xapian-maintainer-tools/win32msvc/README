README for Xapian/Visual C++ Tools
==================================
2011-01-07 for Xapian 1.2.4

Make files, some extra headers and associated tools for compiling Xapian on
Win32 using the Microsoft Visual C++ compilers. This is particularly useful
if you're using the Python for Windows binary distribution as this is also
compiled with Visual C++; otherwise the only way to make the Python
bindings work is to recompile Python with Cygwin (Xapian compiles happily
with Cygwin as standard).

The project was started by Ulrik Petersen, and continued by Charlie Hull of
Lemur Consulting who is now maintaining it.

If you have any feedback, bug reports or comments about these files,
please DO NOT contact Ulrik, but please contact us at Lemur Consulting
www.lemurconsulting.com

Runtime requirements
====================

If you download a prebuilt version, you need:

- Microsoft Windows 2000 or later (NT-based; 95/98/ME won't work)

- It will usually be necessary to install the MSVC 2005 redistributable from
  http://www.microsoft.com/downloads/details.aspx?familyid=32BC1BEE-A3F9-4C13-9C99-220B62A191EE&displaylang=en

What works?
===========

- The Python, PHP, C#, Ruby bindings (but see below for the latter)
- The xapian-compact, xapian-check, xapian-inspect binaries
- The xapian-progsrv and xapian-tcpsrv binaries
- The delve, quest and copydatabase binaries
- The Xapian example binaries
- The testsuite, which is run as part of the makefile in tests/, *but* see below regarding the tests
- omega, omindex and scriptindex (although these may have difficulty parsing
  Windows directory paths)

What doesn't work - Xapian core
===============================

- Warning: unlike Unix, windows does not usually allow open files to be
  deleted.  This caused a serious problem with releases of Xapian on
  Windows before version 0.9.10, potentially leading to database
  corruption.  This is believed to be fixed in version 0.9.10, but there
  remains a restriction that you must close databases before deleting the
  folder (and the files in the folder) containing the database.  We do not
  expect this restriction to be lifted in future releases - equally, we do
  not expect it to cause serious problems.

- To get round the fact that MSVC doesn't do dependency checking correctly,
  we use an external program 'xapdep'. Each Makefile use the -showIncludes switch of the MSVC
  compiler to generate a list of includes as deps.d, which xapdep then processes and inserts
  into the Makefile in the various subdirectories. An unfortunate side effect is that
  details of any compilation errors may be hidden: if you see:

  NMAKE : fatal error U1077: 'cl.exe' : return code '0x2'

  this signifies a compilation error while dependencies were being checked. Look at the file
  deps.d in the appropriate folder to see what actually caused the failure.
  
- The 'replicate3' test currently fails on Windows for the Flint database. 
This may be due to the test itself or the replication subsystem, so at 
present we do not advise the use of the replication system on Windows when using a 
Flint database. The test also hangs forever with the Brass database: if you want to run all 
the other tests (with 'nmake check') you can comment out lines 470-512 of api_replicate.cc.

- The 'compactstub3' and 'compactstub4' tests fail on Windows. If you want to run all 
the other tests (with 'nmake check') you can comment out lines 361-383 and 389-413 of api_compact.cc.

What doesn't work - Xapian bindings
===================================

- The ruby bindings fail one of the tests, see http://trac.xapian.org/ticket/478 for
more details. We have opted to package them anyway (see win32_bindings_ruby.mak for how)

- Any other bindings. Java-Swig bindings do not work for SVN HEAD and the 
  1.2.0 series (note that the Java-Swig bindings are still rather alpha, 
  and may be subject to significant API change). See http://trac.xapian.org/ticket/474
      
TODOS
=====

Other bindings

How do I get the released sources?
==================================

You will need an archive tool which can uncompress .zip and .tgz archives.
A free tool which is suitable for this purpose is available from
http://www.7-zip.org/.

1. Download and unpack Xapian, the Xapian bindings and Omega to the same
   folder. You should have a structure like this (the top level folder
   doesn't have to be called 'xapian'):

xapian\
xapian\xapian-core-0.x.y
xapian\xapian-bindings-0.x.y
xapian\omega-0.x.y

where x and y are revision numbers.

2. Download win32.zip for the appropriate Xapian release from Lemur
   Consulting and place in xapian\xapian-core-0.x.y.
3. Extract the files to the win32/ folder (usually, right click on the ZIP
   file and select Extract Here).
4. IMPORTANT - Edit config.mak to make sure the various individual mak
   files end up in the right folders, and add the correct paths for any
   other binding languages. You may also need to edit the
   win32_applications_omega.mak and win32_bindings_python.mak files, and
   the .mak files for any other bindings. Note that all the .mak files in
   the win32/ folder are automatically copied to the correct places at the
   beginning of the build process, so edit them here rather than where they
   end up.

How do I get the latest development sources?
============================================

The development sources are held in a revision control system called
subversion, so you will first need a subversion client installed to build
the latest development sources.  Details of how to access the xapian
repository are available at http://xapian.org/bleeding

When you check out the xapian sources, you will have a slightly different
structure than that derived from the release tarballs:

xapian\xapian-core
xapian\xapian-bindings
xapian\xapian-applications
xapian\xapian-maintainer-tools
xapian\swig

After checking out the sources, copy the contents of the
"xapian\xapian-maintainer-tools\win32msvc" directory to a new directory named
"xapian\xapian-core\win32". Run the build from this latter directory.

You will then need to edit "xapian\xapian-core\win32\config.mak" as
described in step 4 in the previous section ("How do I get the released
sources?").

What do I need to compile it?
=============================

You will need:

- Windows 2000 or later (NT-based; 95/98/ME won't work)

- Visual C++ 7.0 (.Net 2003/2005). In particular:
  - cl.exe
  - link.exe
  - lib.exe
  - nmake.exe

  OR...

- You can also compile with
Visual C++ 2005/2008 Express Edition available without cost from:
http://msdn.microsoft.com/vstudio/express/visualc/default.aspx

  OR...

- You can also compile with
Visual C++ Toolkit 2003, if you can find it (Microsoft don't distribute this any more) and in this case you'll need the .NET 1.1 SDK:
http://www.microsoft.com/downloads/details.aspx?FamilyID=9b3a2ca6-3647-4070-9f41-a333c6b9181d&displaylang=en
and also to create or download msvcprt.lib, details on how to do this are here:
http://root.cern.ch/root/Procedure/Procedure%20to%20install%20the%20free%20Microsoft%20Visual%20C.htm

If you *don't* use Visual C++ 7.0 then you'll also need the Platform SDK:
http://www.microsoft.com/msdownload/platformsdk/sdkupdate/
 the "Windows® Server 2003 SP1 Platform SDK" is recommended.
which contains headers, libs and NMAKE.

- Perl is used to pre-build some of the source files. Install a Win32 binary from ActiveState (http://www.activestate.com/Products/ActivePerl/) and modify config.mak appropriately (PERL_DIR)

- You will also need the ZLIB library. Download and unzip the binary Windows package
http://www.zlib.net/zlib123-dll.zip
and make sure the path to where you downloaded it matches the path in config.mak (ZLIB_DIR).

- For Omega you will need the PCRE library. Download from 
http://gnuwin32.sourceforge.net/downlinks/pcre.php
(we have used pcre-7.0.exe)
and make sure the path to where you downloaded it matches the path in config.mak (PCRE_DIR).

- To compile the Python bindings you need Python from www.python.org. Make sure the paths to this are set correctly in config.mak (PYTHON_DIR). 
Note that the build system supports Python 2.4, 2.5, 2.6  and 2.7 currently, you must specify which you require.

- To compile the PHP bindings for PHP 5.2 you need:

  a. the PHP sources from www.php.net (we have used 5.2.1).
  b. the built sources from a Windows binary distribution
  c. to edit config.mak to use the correct paths to PHP 5.2 and to select the right version of PHP to build.
  
- To compile the PHP bindings for PHP 5.3 you need to:

  a. follow the instructions at http://wiki.php.net/internals/windows/stepbystepbuild
  b. to edit config.mak to use the correct paths to PHP 5.3 and to select the right version of PHP to build.

- To compile the Java-swig bindings you need the Java SE Development Kit from http://java.sun.com/javase/downloads/index.jsp (we have used the 1.6.0_05 release). Make sure the paths to this are set correctly in config.mak (JAVA_DIR)

- To compile the Ruby bindings you will need the one-click installer for Ruby 1.8.6 from http://www.ruby-lang.org/en/downloads/ - config.mak has settings that will work if this is installed at the default location of c:\ruby. Note that you will have to change the "!=" in the first line of c:\ruby\lib\ruby\1.8\i386-mswin32\config.h
to "<=" if using a Visual C++ older than 5.0.

- To compile the C# bindings you will need the .NET 1.1 SDK (see above) and Visual C# (the Express 2005 edition works, available from:
http://www.microsoft.com/express/2005/). Note that the bindings are produced as two separate DLLs: your C# program will need to import the right one,
see how the Smoketest is built in win32_bindings_csharp.mak for details.

- SWIG is used for recompiling the bindings, but this is done by the Xapian core maintainers, so you shouldn't need to do this unless you are building from the development sources, in which case you'll have to use the version of SWIG in the Xapian repository. To build this follow the instructions at
xapian\swig\Doc\Manual\Windows.html
(you'll need to install MinGW).
To make sure the bindings are rebuilt use the following when building in the bindings folders:

nmake SWIGBUILD=1

Make sure the paths to this are set correctly in config.mak (SWIG)

How do I compile it from a command prompt?
==========================================

1a) Run SetEnv.cmd in the Platform SDK to make sure the SDK's paths are added to your environment. For the "Windows Server 2003 SP1 Platform SDK", installed to the default folder, use the following command:
C:\Program Files\Microsoft Platform SDK\SetEnv.cmd" /RETAIL

1b) Run vcvars32.bat or equivalent for the C compiler to make sure the paths to the compiler are added to your environment. This may be listed as 'Command Prompt' in the Programs menu entry.
For Visual C++ 2005 Express Edition installed to the default folder, use the following command:
C:\Program Files\Microsoft Visual Studio 8\Common7\Tools\vsvars32.bat

1c) If you are using the Express compiler, you'll need to add an include path:
set INCLUDE=%INCLUDE%;%MSSDK%\Include\mfc

1d) If you are using the 2003 Toolkit compiler, and you've thus installed the .NET 1.1 SDK, you'll need to add a library path:
set LIB=%LIB%;C:\Program Files\Microsoft Visual Studio .NET 2003\Vc7\lib;

--------
NOTE: It's easiest to automate the preceding steps. To do this, put the commands above (or the equivalent on your system) into a text file with the .cmd extension (let's call it 'xapenv.cmd'). You could also add a 'cd' to wherever your xapian code is located. Then right-click on your desktop, select New, Shortcut, and find C:\Windows\System32\cmd.exe. Right-click on the new shortcut and select Properties. Edit the Target field to read:
%SystemRoot%\system32\cmd.exe /a /k c:\xapenv "%1"
You can now just run the shortcut to open a command line window ready for building Xapian.
--------

To build the Xapian core:

2) cd to xapian-core-0.x.y\win32 or the appropriate path on your system.
3) edit config.mak to suit your environment
4) nmake CLEAN
either:
5a) nmake CHECK
   - to build and run the test suite
5b) nmake
   - to just build.
    
**NOTE: if you get the following error:

    NMAKE : fatal error U1077: 'cl.exe' : return code '0x2'

    this signifies a compilation error while dependencies were being checked. Look at the file
    deps.d in the appropriate folder to see what actually caused the failure.

To build the bindings (do steps 1-5 above first to make sure you have Xapian built):

6) cd to xapian-core-0.x.y\win32 or the appropriate path on your system.
7) edit 
  win32_bindings_python.mak
  win32_bindings_php.mak
  win32_bindings_ruby.mak 
  win32_bindings_java-swig.mak
  win32_bindings_csharp.mak
    to suit your environment (XAPIAN_CORE_REL_PYTHON or similar)
8) nmake COPYMAKFILES
   - to make sure this change is copied to the build folder (this is also done by a rebuild of Xapian)
9) cd to ..\..\xapian-bindings-0.x.y\ and then the binding folder (php, python etc.)
10) nmake CLEAN
11a) nmake CHECK
   - to build and run the test suite
11b) nmake
   - to just build.
11c) nmake DIST
   - to build, CHECK and also create a /dist subfolder containing a binary distribution, documentation and examples
11d) nmake SWIGBUILD=1
   - to build from the development sources using SWIG (see above)

11e) For Python only, add an extra flag before CLEAN, CHECK or DIST:
    nmake PYTHON_VER=24 ..
 to make bindings for Python 2.4 or
    nmake PYTHON_VER=25 ..
 etc.
 
11f) For Ruby only, use
    nmake INSTALL
 to install the bindings to the standard Ruby library folder.
 
11g) For PHP only, use 
    nmake PHP_VER=52
  to make bindings for PHP 5.2.x, or
    nmake PHP_VER=53
  to make bindings for PHP 5.3.x

 To build *all* the bindings (usually only done by those distributing binaries):

12) cd to xapian-core-0.x.y\win32 or the appropriate path on your system.
13) nmake MAKEALLBINDINGS
    - this will build Xapian if not already available. You will need build environments for all
    the bindings described above available.

 To build Omega (do steps 1-5 above first to make sure you have Xapian built):

14) edit win32_applications_omega.mak to suit your environment (XAPIAN_CORE_REL_OMEGA)
15) nmake COPYMAKFILES
   - to make sure this change is copied to the build folder
16) cd to ..\..\omega-0.x.y
17) nmake
   - to build.

The binaries end up in xapian-core-0.x.y\win32\Release or \Debug if you are building a Debug build (see below). 
If you build using nmake DIST you will see a further /dist subfolder in each bindings's folder, containing 
 a binary distribution, documentation and examples.
 
There is also a Python file designed to package up all the bindings for a particular release. To run this,
install Python 2.4 - 2.6 and :

18) python makebinaries.py x.y.z

where x.y.z is the release number, e.g. 1.0.14

Debug builds
===========

To compile Debug builds, run the build process with the following switch:

nmake DEBUG=1

This affects some lines in config.mak.

If you would like verbose Xapian debugging enabled, add -D XAPIAN_DEBUG_VERBOSE to the CPPFLAGS_EXTRA define in config.mak. You will also need to set up correct environment variables so the Xapian debugger knows where to write its log file. Type the following at a command prompt:

set XAPIAN_DEBUG_LOG=c:/xap.log
set XAPIAN_DEBUG_FLAGS=-1

This will create a log file at c:/xap.log and set the debugging bitmask to show all possible messages. To filter out some of the messages, look at om_debug_types in omdebug.h and set XAPIAN_DEBUG_FLAGS accordingly. For example, to show only OM_DEBUG_REMOTE and OM_DEBUG_API messages, you will need a bitmask according to their position in the table: 1000001000000 in binary (4160 decimal), so:

set XAPIAN_DEBUG_FLAGS=4160

Using the Visual C++ IDE
========================

It is also possible to manually create a project in the Visual C++ Express Edition IDE. To build part of Xapian in the IDE you should add to the project all the .cc files in the api, backends, common, expand, languages, matcher, net, queryparser and unicode subfolders of xapian-core, then add the file containing the main loop for whatever you are building (for example if you're building xapian-check, add xapian-check.cc). You will need to examine config.mak to see what include paths to use and any other necessary compiler switches.


Please send any bugfixes to Charlie Hull at Lemur Consulting:
www.lemurconsulting.com


