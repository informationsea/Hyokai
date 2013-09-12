Hyokai
==========

About
-----

Hyokai is database viewer for big data analysis.
Easy to import from tab-separated values or CSV and export to them.

![Screen Shot](http://informationsea.info/apps/tableview/img/screenshot2.png)

Downloads
---------

* [Source code](https://github.com/informationsea/Hyokai)
* [Binaries](http://informationsea.info/apps/tableview/downloads.html)

Requirements
------------

* Mac OS X 10.5 Leopard or later
* Windows XP or later

### Build Requirements

* Qt 4.8 or 5.0
* Qt Creator (Recommend)

Supported Databases
-------------------

* SQLite3 (built in)
* MySQL
* PosgreSQL

How to build
------------

### Release build

#### Mac OS X

1. Build Qt with configure options `./configure -no-qt3support -no-declarative-debug -no-declarative -no-scripttools -no-script -no-javascript-jit -no-svg -no-phonon-backend -no-audio-backend -no-phonon -no-multimedia -no-webkit -system-sqlite -qt-sql-sqlite -nomake examples -nomake demos -nomake docs -opensource -prefix /usr/local/Trolltech/Qt-4.8.4-tableview && make`
2. Install Qt with `sudo make install`
3. move to `build-scripts/script-for-mac`
4. run `build.sh`

`qmake` should be included into `$PATH`

#### Windows

1. Install pre-built Qt 4.8
2. Open Qt command prompt
3. move to `build-scripts/script-for-windows`
4. run `build.bat`

Please read `build-scripts/script-for-windows/README.md` to learn more.

### Debug build

1. Build Qt if you are using OS X or other unix-like systems with `-qt-sql-sqlite -system-sqlite` options
2. Open `src/Hyokai.pro` with Qt Creator
3. Build it

Authors
-------

* Programming & Icon of application - Y.Okamura
* Programming - S.T
* Toolbar icons - Tango Desktop Project

License
-------

GPL version 3 or later

