Table View build script for Windows
===================================

Run "build.bat" to build TableView.exe for windows.
You have to install Qt SDK with Visual Studio and run build.bat in Qt command prompt.
If you want to use Qt SDK with MinGW, please replace "nmake" with "make" in build.bat

Qt 4.8.4 with Visual Studio 2010
--------------------------------

You have to include these files in below to distribute.

* QtCore4.dll
* QtGui4.dll
* QtSql4.dll

They are found in C:\Qt\4.8.4\bin

### Installer

Run "installer.nsi" to build the installer. TableView.exe should be built with Qt 4.8.4 before making the installer.


Qt 5.0.1 with Visual Studio 2010
--------------------------------

You have to include these files in below to distribute.

* D3DCompiler_43.dll
* icudt49.dll
* icuin49.dll
* icuuc49.dll
* libEGL.dll
* libGLESv2.dll
* Qt5Core.dll
* Qt5Gui.dll
* Qt5Sql.dll
* Qt5Widgets.dll

They are found in C:\Qt\Qt5.0.1\5.0.1\msvc2010\bin

