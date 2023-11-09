#ifndef ANNO_POOL_H
#define ANNO_POOL_H

#include <stdlib.h>
#include "htslib/vcf.h"
#include "htslib/hts.h"

#define RECORDS_PER_CHUNK 1000
#define CHUNK_MAX_GAP 10000

struct anno_pool {
    // maximum number of records
    int m;
    int n_reader;   
    bcf1_t **readers;

    // init records per chunk
    int i_chunk;
    int n_chunk;
    int curr_start;
    int curr_end;
    bcf1_t *curr_line; // point to top of each chunk in the readers
    
    void *arg;
};

extern struct anno_pool *anno_reader(htsFile *fp, bcf_hdr_t *hdr, int n_record);

extern void update_chunk_region(struct anno_pool *pool);
#endif
