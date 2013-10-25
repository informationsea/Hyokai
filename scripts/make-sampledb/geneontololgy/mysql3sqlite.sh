#!/bin/sh

cat $1 |\
sed -e 's/`/"/g'|sed -e 's/int([[:digit:]]*)/INTEGER/g'|\
sed -e 's/[[:alpha:]]* KEY.*$//g'|\
sed -e 's/AUTO_INCREMENT/PRIMARY KEY AUTOINCREMENT/g'
