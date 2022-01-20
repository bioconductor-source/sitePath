# sitePath: finding phylogeny-dependent fixation and parallel mutations

## Installation

[R programming language](https://cran.r-project.org/) \>= 4.1.0 is
required to use `sitePath`.

The stable release is available on
[Bioconductor](https://bioconductor.org/packages/sitePath/).

``` r
if (!requireNamespace("BiocManager", quietly = TRUE))
    install.packages("BiocManager")

BiocManager::install("sitePath")
```

The installation from [GitHub](https://github.com/wuaipinglab/sitePath/)
is in experimental stage but gives the newest feature:

``` r
if (!requireNamespace("remotes", quietly = TRUE))
    install.packages("remotes")

remotes::install_github("wuaipinglab/sitePath")
```

## Data preparation

You need a *tree* and a *MSA* (multiple sequence alignment) file and the
sequence names have to be matched!

``` r
library(sitePath) # Load the sitePath package

# The path to your tree and MSA files
tree_file <- system.file("extdata", "ZIKV.newick", package = "sitePath")
alignment_file <- system.file("extdata", "ZIKV.fasta", package = "sitePath")


tree <- read.tree(tree_file) # Read the tree file into R
align <- read.alignment(alignment_file, format = "fasta") # Read the MSA file into R
```

## Run analysis

`Nmin` and `minSNP` are the respective parameters for finding fixation
and parallel sites (18 and 1 are used as an example for this dataset).
The default values will be used if you don’t specify them.

``` r
options(list("cl.cores" = 1)) # Set this bigger than 1 to use multiprocessing

paraFix <- paraFixSites(tree, alignment = align, Nmin = 18, minSNP = 1) # Run analysis to find fixation and parallel sites
paraFix
```

    ## This is a 'paraFixSites' object
    ## 
    ## fixation sites:
    ## 139, 894, 2074, 2086, 2634, 3045, 988, 1143, 2842, 3398, 107, 1118, 3353
    ## 
    ## parallel sites:
    ## 105, 2292, 1264, 918, 1226, 1717, 988, 2611, 2787, 2749, 3328, 3162, 1857, 2445, 358, 1404, 3046, 791, 1180, 1016, 1171, 1327, 3076, 106, 2357, 916, 1303, 969, 573, 2909, 2122, 940
    ## 
    ## paraFix sites:
    ## 988

## Fixation sites

Use `allSitesName` and set `type` as “fixation” to retrieve fixation
sites name

``` r
allSitesName(paraFix, type = "fixation")
```

    ##  [1] "139"  "894"  "2074" "2086" "2634" "3045" "988"  "1143" "2842" "3398"
    ## [11] "107"  "1118" "3353"

Use `plotFixationSites` to view fixation sites

``` r
plotFixationSites(paraFix) # View all fixation sites on the tree
```

![](inst/plot_fixSites-1.png)<!-- -->

``` r
plotFixationSites(paraFix, site = 139) # View a single site
```

![](inst/plot_fixSites-2.png)<!-- -->

## Parallel sites

Use `allSitesName` and set `type` as “parallel” to retrieve parallel
sites name

``` r
allSitesName(paraFix, type = "parallel")
```

    ##  [1] "105"  "2292" "1264" "918"  "1226" "1717" "988"  "2611" "2787" "2749"
    ## [11] "3328" "3162" "1857" "2445" "358"  "1404" "3046" "791"  "1180" "1016"
    ## [21] "1171" "1327" "3076" "106"  "2357" "916"  "1303" "969"  "573"  "2909"
    ## [31] "2122" "940"

Use `plotParallelSites` to view parallel sites

``` r
plotParallelSites(paraFix) # View all parallel sites on the tree
```

![](inst/unnamed-chunk-1-1.png)<!-- -->

``` r
plotParallelSites(paraFix, site = 105) # View a single site
```

![](inst/unnamed-chunk-1-2.png)<!-- -->

## Read more

The above uses wrapper functions but the analysis can be dissembled into
step functions (so you can view the result of each step and modify
parameters). Click
[here](https://wuaipinglab.github.io/sitePath/articles/sitePath.html)
for a detailed breakdown of the functionality.

## Getting help

Post on Bioconductor [support site](https://support.bioconductor.org/)
if having trouble using `sitePath`. Or open an
[issue](https://github.com/wuaipinglab/sitePath/issues/new?assignees=&labels=&template=bug_report.md&title=)
if a bug is found.
