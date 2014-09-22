nmake distclean
qmake -config release ..\..\src
nmake

if defined FrameworkVersion64 move release\Hyokai.exe release\Hyokai_x64.exe
if not defined FrameworkVersion64 move release\Hyokai.exe release\Hyokai_x86.exe
