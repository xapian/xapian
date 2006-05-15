@echo off
rem 
rem copies all the individual mak files into the various source folders
rem Should be run from the win32 folder!
rem
copy  win32_api.mak ..\api\win32.mak
copy  win32_backends.mak ..\backends\win32.mak
copy  win32_backends_flint.mak ..\backends\flint\win32.mak
copy  win32_backends_inmemory.mak ..\backends\inmemory\win32.mak
copy  win32_backends_multi.mak ..\backends\multi\win32.mak
rem copy  win32_backends_muscat36.mak ..\backends\muscat36\win32.mak
rem copy  win32_backends_net.mak ..\backends\net\win32.mak
copy  win32_backends_quartz.mak ..\backends\quartz\win32.mak
copy  win32_bin.mak ..\bin\win32.mak
copy  win32_common.mak ..\common\win32.mak
copy  win32_examples.mak ..\examples\win32.mak
copy  win32_getopt.mak ..\getopt\win32.mak
copy  win32_languages.mak ..\languages\win32.mak
copy  win32_matcher.mak ..\matcher\win32.mak
copy  win32_queryparser.mak ..\queryparser\win32.mak
copy  win32_tests.mak ..\tests\win32.mak
copy  win32_testsuite.mak ..\testsuite\win32.mak

echo ..Copied all core make files.

if not exist ..\..\xapian-bindings-0.9.6\ goto failbindings
copy win32_bindings_python.mak ..\..\xapian-bindings-0.9.6\python\win32.mak
echo ..Copied all bindings make files.
goto copyapplications
:failbindings
echo ERROR: The bindings are not installed. Make files cannot be copied.

:copyapplications
if not exist ..\..\omega-0.9.6\ goto failomega
copy win32_applications_omega.mak ..\..\omega-0.9.6\win32.mak
copy config.mak ..\..\omega-0.9.6
copy config.h.omega.win32 ..\..\omega-0.9.6\config.h
echo ..Copied all applications files.
goto end
:failomega
echo ERROR: Omega is not installed. Make files cannot be copied.
goto end

:end 
echo ..Make file copy is done.
