call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
set PATH=C:\Qt\5.9.7\msvc2015_64\bin;%PATH%;C:\Program Files (x86)\Windows Kits\8.1\bin\x64
echo %PATH%
build.bat
