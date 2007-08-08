if not exist xapian-core\win32\. md xapian-core\win32
rem Buildbot helper to prepare for a Xapian windows build
if not exist xapian-core\win32\. md xapian-core\win32
xcopy xapian-maintainer-tools\win32msvc\* xapian-core\win32 /y /s
rem Set up environment for Visual C++ 2005 Express Edition
call "C:\Program Files\Microsoft Visual Studio 8\Common7\Tools\vsvars32.bat"
rem Compile the makedepend tool
cd xapian-core\win32\makedepend
nmake -f makedepend.mak
copy makedepend.exe ..
cd ..\..\..
