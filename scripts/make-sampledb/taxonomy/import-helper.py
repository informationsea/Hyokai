#!/usr/bin/env python

import argparse
import csv
import sqlite3
import sys

def _main():
    parser = argparse.ArgumentParser(description="Import Taxonomy")
    parser.add_argument('db', default='taxonomy.sqlite3', nargs='?')
    parser.add_argument('names', default='names.dmp', nargs='?', type=argparse.FileType('r'))
    parser.add_argument('citations', default='citations.dmp', nargs='?', type=argparse.FileType('r'))
    options = parser.parse_args()

    print >>sys.stderr, 'loading names'
    db = sqlite3.connect(options.db)
    names_data = options.names.read()
    names_lines = names_data.split('\t|\n')
    for line in names_lines:
        elements = line.split('\t|\t')
        #print elements
        if len(elements) != 4:
            continue
        elements[0] = int(elements[0])
        db.execute('INSERT INTO taxonomy_names VALUES(?,?,?,?)', elements)

    print >>sys.stderr, 'loading citations'
    citations_data = options.citations.read()
    citations_lines = citations_data.split('\t|\n')
    for line in citations_lines:
        elements = [unicode(x, 'ISO8859') for x in line.split('\t|\t')]
        if len(elements) != 7:
            print len(elements)
            continue
        #print elements
        elements[5] = elements[5].replace('\\n','\n').replace('\\t','\t').replace('\\"','"').replace('\\\\','\\')
        db.execute('INSERT INTO taxonomy_citations VALUES(?,?,?,?,?,?,?)', elements)

    db.commit()
    
if __name__ == '__main__':
    _main()
