#!/bin/sh

if [ ! -e gene_info ]; then
    curl -O ftp://ftp.hgc.jp/pub/mirror/ncbi/gene/DATA/gene_info.gz
    gunzip gene_info.gz
fi

function importtax() {
    SUFFIX=$1
    TAXID=$2
    echo $1 $2

    if [ ! -e gene_info-$SUFFIX ];then
        egrep "^$TAXID	" gene_info > gene_info-$SUFFIX
    fi
    
    rm gene_info-$SUFFIX.sqlite3 2> /dev/null

    sqlite3 gene_info-$SUFFIX.sqlite3 'CREATE TABLE gene_info(tax_id INTEGER,GeneID INTEGER,Symbol TEXT,LocusTag TEXT,Synonyms TEXT,dbXrefs TEXT,chromosome TEXT,map_location TEXT,description TEXT,type_of_gene TEXT,Symbol_from_nomenclature_authority TEXT,Full_name_from_nomenclature_authority TEXT,Nomenclature_status TEXT,Other_designations TEXT,Modification_date INTEGER);'

    sqlite3 gene_info-$SUFFIX.sqlite3 <<EOF
.separator "	"
.import gene_info-$SUFFIX gene_info
CREATE INDEX gene_info__tax_id__index on gene_info(tax_id);
CREATE INDEX gene_info__GeneID__index on gene_info(GeneID);
CREATE INDEX gene_info__Symbol__index on gene_info(Symbol);
CREATE INDEX gene_info__LocusTag__index on gene_info(LocusTag);
CREATE INDEX gene_info__chromosome__index on gene_info(chromosome);
CREATE INDEX gene_info__map_location__index on gene_info(map_location);
CREATE INDEX gene_info__type_of_gene__index on gene_info(type_of_gene);
CREATE INDEX gene_info__Symbol_from_nomenclature_authority__index on gene_info(Symbol_from_nomenclature_authority);
CREATE INDEX gene_info__Nomenclature_status__index on gene_info(Nomenclature_status);
CREATE INDEX gene_info__Modification_date__index on gene_info(Modification_date);
EOF

    rm gene_info-$SUFFIX.zip 2> /dev/null
    7z a gene_info-$SUFFIX.zip gene_info-$SUFFIX.sqlite3
}

# taxonomy ids                  
# 9606  Human                   
# 10090 Mouse                   
# 7227  Drosophila melanogaster 
# 3702  Arabidopsis thaliana    
# 4577  Zea mays (Maize)        
# 7955  Danio rerio (Zebra fish)
# 6239  Caenorhabditis elegans  
# 4932  Saccharomyces cerevisiae

importtax hsa 9606
importtax mmu 10090
importtax dme 7227
importtax ath 3702
importtax dre 1955
importtax cel 6239

