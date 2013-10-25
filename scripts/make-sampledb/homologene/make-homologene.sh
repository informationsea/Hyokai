#!/bin/sh

if [ ! -e homologene.data ];then
    curl -O ftp://anonymous@ftp.ncbi.nih.gov/pub/HomoloGene/current/homologene.data
fi

rm homologene.sqlite3 2> /dev/null
sqlite3 homologene.sqlite3 <<EOF
CREATE TABLE homologene(HID INTEGER, tax_id INTEGER, GeneID INTEGER, GeneSymbol TEXT, Protein_gi INTEGER, Protein_accession TEXT);
.separator "	"
.import "./homologene.data" homologene
CREATE INDEX homologene__HID__index ON homologene(HID);
CREATE INDEX homologene__tax_id__index ON homologene(tax_id);
CREATE INDEX homologene__GeneID__index ON homologene(GeneID);
CREATE INDEX homologene__GeneSymbol__index ON homologene(GeneSymbol);
CREATE INDEX homologene__Protein_gi__index ON homologene(Protein_gi);
CREATE INDEX homologene__Protein_accession__index ON homologene(Protein_accession);
EOF

7z a homologene.sqlite3.zip homologene.sqlite3

