Table View
==========

Table View is simple SQLite database viewer

How to build
------------

1. Build Qt Library
     * Recommend qt configure options to build are listed in below
         * Mac OS X `./configure -no-qt3support -no-declarative-debug -no-declarative -no-scripttools -no-script -no-javascript-jit -no-svg -no-phonon-backend -no-audio-backend -no-phonon -no-multimedia -no-webkit -system-sqlite -qt-sql-sqlite -nomake examples -nomake demos -nomake docs -opensource -prefix /usr/local/Trolltech/Qt-4.8.4-tableview`
     * I don't recommend to use pre-build packages because it may fail to build.
2. run `qmake`
3. run `make`

License
-------

GPL3 or later


Authors
-------

* Programming & Icon of application - Y.Okamura
* Toolbar icons - Tango Desktop Project

