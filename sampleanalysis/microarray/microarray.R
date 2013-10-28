source("http://bioconductor.org/biocLite.R")
biocLite(c("GEOquery", "affy", "mouse4302.db"))

library(GEOquery)
library(affy)
library(mouse4302.db)

mouse.probe.keys <- mouse4302ENTREZID
mapped_probes <- mappedkeys(mouse.probe.keys)
probe2gene <- as.data.frame(mouse.probe.keys[mapped_probes])
write.csv(probe2gene, "probe2gene.csv")

geo.data <- getGEO("GSE46443")
geo.exprs <- exprs(geo.data[[1]])
write.csv(geo.exprs, "expressiondata.csv")


