if defined FrameworkVersion64 (
	mkdir build64
	pushd build64
) else (
	mkdir build32
	pushd build32
)

del /Q /S deploy
rmdir /S /Q deploy

nmake distclean
qmake -config release ..\..\..\src
nmake

mkdir deploy
copy release\Hyokai.exe deploy
pushd deploy
windeployqt Hyokai.exe

mkdir plugins
move sqldrivers plugins
move imageformats plugins

popd

popd
