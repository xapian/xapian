rem Set up environment for Visual C++ 2005 Express Edition
call "C:\Program Files\Microsoft Visual Studio 8\Common7\Tools\vsvars32.bat"

set PYTHON_DIR=C:\Python25
set PERL_DIR=C:\Perl\bin
set ZLIB_DIR=D:\zlib123-dll

set EXTRAPATHS=PYTHON_DIR=%PYTHON_DIR% PERL_DIR=%PERL_DIR% ZLIB_DIR=%ZLIB_DIR% ZLIB_INCLUDE_DIR=%ZLIB_DIR%\include ZLIB_LIB_DIR=%ZLIB_DIR%\lib ZLIB_BIN_DIR=%ZLIB_DIR%
set CHECKEXTRAPATHS=XAPIAN_TESTSUITE_OUTPUT=plain %EXTRAPATHS%

rem Build "makedepend" and put it in place
cd makedepend
nmake -f makedepend.mak
copy makedepend.exe ..
cd ..

nmake COPYMAKFILES

rem Compile and test each module
nmake %EXTRAPATHS%
nmake CHECK %CHECKEXTRAPATHS%

cd ....\xapian-bindings\python
nmake %EXTRAPATHS%
nmake CHECK %CHECKEXTRAPATHS%

cd ....\xapian-bindings\perl
nmake %EXTRAPATHS%
nmake CHECK %CHECKEXTRAPATHS%

cd ..\..\xapian-omega
nmake %EXTRAPATHS%
nmake CHECK %CHECKEXTRAPATHS%
