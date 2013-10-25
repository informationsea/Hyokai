#!/bin/sh

if [ ! -e taxdump.tar.gz ];then
    curl -O ftp://anonymous@ftp.hgc.jp/pub/mirror/ncbi/taxonomy/taxdump.tar.gz
    tar xzf taxdump.tar.gz
fi

rm taxonomy.sqlite3 2> /dev/null

for i in *.dmp; do
    echo $i
    if [ -e ${i%.dmp}.txt ]; then
        if [ ${i%.dmp}.txt -nt $i ];then
            continue
        fi
    fi
    perl -pe 's/\t\|//g' $i > ${i%.dmp}.txt
done

sqlite3 taxonomy.sqlite3 <<EOF
.separator "	"
BEGIN TRANSACTION;
CREATE TABLE taxonomy_division(division_id INTEGER PRIMARY KEY, division_cde TEXT, division_name TEXT, comments TEXT);
.import "./division.txt" taxonomy_division
CREATE INDEX taxonomy_division__division_code__index ON taxonomy_division(division_cde);
CREATE INDEX taxonomy_division__division_name__index ON taxonomy_division(division_name);
CREATE TABLE taxonomy_gencode(genetic_code_id INTEGER PRIMARY KEY, abbreviation TEXT, name TEXT, cde TEXT, starts TEXT);
.import "./gencode.txt" taxonomy_gencode
CREATE INDEX taxonomy_gencode__name__index ON taxonomy_gencode(name);
CREATE INDEX taxonomy_gencode__cde__index ON taxonomy_gencode(cde);
CREATE INDEX taxonomy_gencode__starts__index ON taxonomy_gencode(starts);
CREATE TABLE taxonomy_merged(old_tax_id INTEGER PRIMARY KEY, new_tax_id INTEGER);
.import "./merged.txt" taxonomy_merged
CREATE INDEX taxonomy_merged__new_tax_id__index ON taxonomy_merged(new_tax_id);
CREATE TABLE taxonomy_dellnodes(tax_id INTEGER PRIMARY KEY);
.import "./delnodes.txt" taxonomy_dellnodes


COMMIT;

CREATE TABLE taxonomy_nodes(tax_id INTEGER PRIMARY KEY, parent_tax INTEGER, rank TEXT, embl_code TEXT, division_id INTEGER REFERENCES taxonomy_divison(division_id), inherited_div_flag INTEGER, genetic_code_id INTEGER REFERENCES taxonomy_gencode(genetic_code_id), inherited_GC_flag INTEGER, mitochondrial_genetic_code_id INTEGER REFERENCES taxonomy_gencode(genetic_code_id), inherited_MGC_flag INTEGER, GenBank_hidden_flag INTEGER, hidden_subtree_root_lag INTEGER, comments TEXT);
.import "./nodes.txt" taxonomy_nodes
CREATE INDEX taxonomy_nodes__parent_tax__index ON taxonomy_nodes(parent_tax);
CREATE INDEX taxonomy_nodes__rank__index ON taxonomy_nodes(rank);
CREATE INDEX taxonomy_nodes__embl_code__index ON taxonomy_nodes(embl_code);
CREATE INDEX taxonomy_nodes__division_id__index ON taxonomy_nodes(division_id);
CREATE INDEX taxonomy_nodes__genetic_code_id__index ON taxonomy_nodes(genetic_code_id);
CREATE INDEX taxonomy_nodes__mitocondrial_genetic_code_id__index ON taxonomy_nodes(mitochondrial_genetic_code_id);

CREATE TABLE taxonomy_names(tax_id INTEGER  REFERENCES taxonomy_nodes(tax_id), name_txt TEXT, unique_name TEXT, name_class TEXT);
-- .import "./names.txt" taxonomy_names
CREATE INDEX taxonomy_names__tax_id__index ON taxonomy_names(tax_id);
CREATE INDEX taxonomy_names__name_txt__index ON taxonomy_names(name_txt);
CREATE INDEX taxonomy_names__unique_name__index ON taxonomy_names(unique_name);
CREATE INDEX taxonomy_names__name_class__index ON taxonomy_names(name_class);

CREATE TABLE taxonomy_citations(cit_id INTEGER PRIMARY KEY, cit_key TEXT, pubmed_id INTEGER, medline_id INTEGER, url TEXT, text_data TEXT, taxid_list TEXT);
CREATE INDEX taxonomy_names__cit_key__index ON taxonomy_citations(cit_key);
CREATE INDEX taxonomy_names__pubmed_id__index ON taxonomy_citations(pubmed_id);
CREATE INDEX taxonomy_names__medline_id__index ON taxonomy_citations(medline_id);

EOF

python import-helper.py
7z a taxonomy.sqlite3.zip taxonomy.sqlite3
