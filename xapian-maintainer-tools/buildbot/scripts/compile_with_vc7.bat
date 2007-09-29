rem Set up environment for Visual C++ 2005 Express Edition
call "C:\Program Files\Microsoft Visual Studio 8\Common7\Tools\vsvars32.bat"

rem Build "makedepend" and put it in place
cd makedepend
nmake -f makedepend.mak
copy makedepend.exe ..
cd ..

nmake COPYMAKFILES

rem Compile and test each module
nmake PERL_DIR=C:\Perl\bin ZLIB_DIR=D:\zlib123-dll
nmake CHECK XAPIAN_TESTSUITE_OUTPUT=plain PERL_DIR=C:\Perl\bin ZLIB_DIR=D:\zlib123-dll

cd ....\xapian-bindings\python
nmake PYTHON_DIR=C:\Python25 PERL_DIR=C:\Perl\bin ZLIB_DIR=D:\zlib123-dll
nmake CHECK XAPIAN_TESTSUITE_OUTPUT=plain PYTHON_DIR=C:\Python25 PERL_DIR=C:\Perl\bin ZLIB_DIR=D:\zlib123-dll

cd ....\xapian-bindings\perl
nmake PERL_DIR=C:\Perl\bin PERL_DIR=C:\Perl\bin ZLIB_DIR=D:\zlib123-dll
nmake CHECK XAPIAN_TESTSUITE_OUTPUT=plain PERL_DIR=C:\Perl\bin  ZLIB_DIR=D:\zlib123-dll

cd ..\..\xapian-omega
nmake PERL_DIR=C:\Perl\bin ZLIB_DIR=D:\zlib123-dll
nmake CHECK PERL_DIR=C:\Perl\bin ZLIB_DIR=D:\zlib123-dll
