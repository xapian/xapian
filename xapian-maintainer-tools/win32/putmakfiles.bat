@echo off
rem 
rem copies all the individual mak files into the various source folders
rem Should be run from the win32 folder!
rem
copy  win32.mak ..\xapian-core-0.9.7\win32.mak
copy  win32_api.mak ..\xapian-core-0.9.7\api\win32.mak
copy  win32_backends.mak ..\xapian-core-0.9.7\backends\win32.mak
copy  win32_backends_flint.mak ..\xapian-core-0.9.7\backends\flint\win32.mak
copy  win32_backends_inmemory.mak ..\xapian-core-0.9.7\backends\inmemory\win32.mak
copy  win32_backends_multi.mak ..\xapian-core-0.9.7\backends\multi\win32.mak
rem copy  win32_backends_muscat36.mak ..\xapian-core-0.9.7\backends\muscat36\win32.mak
rem copy  win32_backends_net.mak ..\xapian-core-0.9.7\backends\net\win32.mak
copy  win32_backends_quartz.mak ..\xapian-core-0.9.7\backends\quartz\win32.mak
copy  win32_bin.mak ..\xapian-core-0.9.7\bin\win32.mak
copy  win32_common.mak ..\xapian-core-0.9.7\common\win32.mak
copy  win32_examples.mak ..\xapian-core-0.9.7\examples\win32.mak
copy  win32_getopt.mak ..\xapian-core-0.9.7\getopt\win32.mak
copy  win32_languages.mak ..\xapian-core-0.9.7\languages\win32.mak
copy  win32_net.mak ..\xapian-core-0.9.7\net\win32.mak
copy  win32_matcher.mak ..\xapian-core-0.9.7\matcher\win32.mak
copy  win32_queryparser.mak ..\xapian-core-0.9.7\queryparser\win32.mak
copy  win32_tests.mak ..\xapian-core-0.9.7\tests\win32.mak
copy  win32_testsuite.mak ..\xapian-core-0.9.7\testsuite\win32.mak

echo ..Copied all core make files.

if not exist ..\xapian-bindings-0.9.7\ goto failbindings
copy win32_bindings_python.mak ..\xapian-bindings-0.9.7\python\win32.mak
echo ..Copied all bindings make files.
goto copyapplications
:failbindings
echo ERROR: The bindings are not installed. Make files cannot be copied.

:copyapplications
if not exist ..\omega-0.9.7\ goto failomega
copy win32_applications_omega.mak ..\omega-0.9.7\win32.mak
copy config.mak ..\omega-0.9.7
copy config.h.omega.win32 ..\omega-0.9.7\config.h
echo ..Copied all applications files.
goto end
:failomega
echo ERROR: Omega is not installed. Make files cannot be copied.
goto end

:end 
echo ..Make file copy is done.

cd ..\xapian-core-0.9.7\
nmake -f win32.mak
cd ..\xapian-bindings-0.9.7\
nmake -f win32.mak
