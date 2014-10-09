#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse
import sqlite3
import collections
import re

def _main():
    parser = argparse.ArgumentParser(description="Import GO term into sqlite3")
    parser.add_argument('go', default='go.obo', type=argparse.FileType('r'), help='default: %(default)s', nargs='?')
    parser.add_argument('-o', '--output', default='goterm.sqlite3', help='default: %(default)s')
    options = parser.parse_args()

    conn = sqlite3.connect(options.output)

    conn.execute('DROP TABLE IF EXISTS dbinfo')
    conn.execute('DROP TABLE IF EXISTS term')
    conn.execute('DROP TABLE IF EXISTS term_info')
    conn.execute('CREATE TABLE dbinfo(key TEXT, value TEXT)')
    conn.execute('CREATE TABLE term(GOID TEXT PRIMARY KEY, name TEXT, namespace TEXT, def TEXT)')
    conn.execute('CREATE TABLE term_info(GOID TEXT, key TEXT, value TEXT)')

    mode = 'DBINFO'
    current_dic = collections.defaultdict(list)

    for line in options.go:
        line = line.rstrip()
        if not line:
            #print current_dic
            register(conn, mode, current_dic)
            current_dic.clear()
        elif line.startswith('[') and line.endswith(']'):
            mode = line[1:-1]
        else:
            elements = line.split(': ', 1)
            current_dic[elements[0]].append(elements[1])
    register(conn, mode, current_dic)

    conn.execute('CREATE INDEX IF NOT EXISTS term__name__index ON term(name)')
    conn.execute('CREATE INDEX IF NOT EXISTS term__namespace__index ON term(namespace)')
    conn.execute('CREATE INDEX IF NOT EXISTS term__def__index ON term(def)')

    conn.execute('CREATE INDEX IF NOT EXISTS term_info__GOID__index ON term_info(GOID)')
    conn.execute('CREATE INDEX IF NOT EXISTS term_info__key__index ON term_info(key)')
    conn.execute('CREATE INDEX IF NOT EXISTS term_info__value__index ON term_info(value)')
    
    conn.commit()

def register(conn, mode, current_dic):
    if mode == 'DBINFO':
        for k, v in current_dic.iteritems():
            for one in v:
                conn.execute('INSERT INTO dbinfo VALUES(?, ?)', [k, one])
        return

    name = current_dic['name'][0] if current_dic['name'] else ''
    namespace = current_dic['namespace'][0] if current_dic['namespace'] else ''
    godef = current_dic['def'][0] if current_dic['def'] else ''

    if mode == 'Term':
        conn.execute('INSERT INTO term VALUES(?, ?, ?, ?)', [current_dic['id'][0], name, namespace, godef])
        for k, v in current_dic.iteritems():
            if k in ('id', 'name', 'namespace', 'def'): continue
            for one in v:
                conn.execute('INSERT INTO term_info VALUES(?, ?, ?)', [current_dic['id'][0], k, one])
    

if __name__ == '__main__':
    _main()
