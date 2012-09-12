#!/bin/sh

# http://www.sqlite.org/cvstrac/wiki?p=LoadableExtensions

if [ `uname` = "Darwin" ]; then # OS X
    gcc -bundle -fPIC -o extensions.sqlext extension-functions.c
elif  [ `uname` = "Linux" ]; then # Linux
    gcc -shared -fPIC -o extensions.sqlext extension-functions.c
fi
