#ifndef ANNO_BED_HEADER
#define ANNO_BED_HEADER
#include <stdio.h>
#include <stdlib.h>
#include <htslib/vcf.h>
#include <htslib/kstring.h>
#include <htslib/khash.h>
#include <htslib/tbx.h>
#include "anno.h"

struct beds_anno_tsv {
    int nfields;
    int *fields;
    // int tid;
    int start;
    int end;
    kstring_t string;
};

struct beds_anno_file {
    int id; // file idx
    htsFile *fp;
    tbx_t *idx;
    char *fname;
    int no_such_chrom;
    int last_id;
    int last_start;
    int last_end;
    int n_cols;
    int m_cols;
    struct anno_col *cols;
    // memory pool    
    int cached, max;
    struct beds_anno_tsv **buffer;

    // set to 0 if records are NOT overlapped, buffer will only be refreshed when record out of range
    int overlapped;
};

struct beds_options {
    int beds_is_inited;
    bcf_hdr_t *hdr_out;
    int n_files;
    int m_files;    
    struct beds_anno_file *files;    
};

// bed format function annotation
// extern int setter_func_region(struct beds_options *opts, bcf1_t *line
extern int beds_options_init(struct beds_options *opts);

extern int beds_options_destroy(struct beds_options *opts);

extern int anno_beds_core(struct beds_options *opts, bcf1_t *line);

extern int beds_database_add(struct beds_options *opts, const char *fname, char *columns);

#endif
