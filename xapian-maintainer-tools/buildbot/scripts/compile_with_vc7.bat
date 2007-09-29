rem Set up environment for Visual C++ 2005 Express Edition
call "C:\Program Files\Microsoft Visual Studio 8\Common7\Tools\vsvars32.bat"

rem Build "makedepend" and put it in place
cd makedepend
nmake -f makedepend.mak
copy makedepend.exe ..
cd ..

rem Compile each module
nmake
cd ..\xapian-omega
nmake
cd ..\xapian-bindings
nmake XAPIAN_TESTSUITE_OUTPUT=plain PERL_DIR=C:\Perl\bin PYTHON_DIR=C:\Python25

rem Run the testsuites
cd ..\xapian-core
nmake check
cd ..\xapian-omega
nmake check
cd ..\xapian-bindings
nmake check
