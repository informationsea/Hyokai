#!/bin/sh

if [ ! -e gene2refseq ]; then
    curl -O ftp://ftp.hgc.jp/pub/mirror/ncbi/gene/DATA/gene2refseq.gz
    gunzip gene2refseq.gz
fi

rm gene2refseq.sqlite3

if [ ! -e gene2refseq_without_header ]; then
    sed -e 1d gene2refseq > gene2refseq_without_header
fi

sqlite3 gene2refseq.sqlite3 <<EOF
CREATE TABLE gene2refseq(tax_id INTEGER, GeneID INTEGER, status TEXT, RNA_nucleotide_accession_version TEXT, RNA_nucleotide_gi TEXT, protein_accession_version TEXT, protein_gi TEXT, genomic_nucleotide_accession_version TEXT, genomic_nucleotide_gi TEXT, start_position_on_the_genomic_accession TEXT, end_position_on_the_genomic_accession TEXT, orientation TEXT, assembly TEXT, mature_peptide_accession_version TEXT, mature_peptide_gi TEXT, Symbol TEXT);
.separator "	"
.import gene2refseq_without_header gene2refseq
CREATE INDEX gene2refseq__tax_id__index ON gene2refseq(tax_id);
CREATE INDEX gene2refseq__GeneID__index ON gene2refseq(GeneID);
CREATE INDEX gene2refseq__RNA_nucleotide_accession_version__index ON gene2refseq(RNA_nucleotide_accession_version);
CREATE INDEX gene2refseq__protein_accession_version__index ON gene2refseq(protein_accession_version);
CREATE INDEX gene2refseq__genomic_nucleotide_accession_version__index ON gene2refseq(genomic_nucleotide_accession_version);
CREATE INDEX gene2refseq__Symbol__index ON gene2refseq(Symbol);
EOF

